#include "app/main.h"
#include "mrnutz/mrnutz_spc_player.h"
#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "cpu_state.h"
#include "snes/snes.h"
#include "snes/ppu.h"
#include "launcher.h"
#include "host_report.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Snes *g_snes;
struct SpcPlayer *g_spc_player;
static int g_curr_fps = 0;

void NORETURN Die(const char *error) {
  host_report_fatal(error);
  fprintf(stderr, "Fatal: %s\n", error);
  exit(1);
}

void MkDir(const char *s) {
#ifdef _WIN32
  _mkdir(s);
#else
  mkdir(s, 0755);
#endif
}

int main(int argc, char **argv) {
  host_report_init("mrnutz", "dev");

  /* Parse command-line arguments */
  const char *dump_dir = NULL;
  int dump_skip = 0, dump_max = 600;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--framedump") == 0 && i+1 < argc) dump_dir = argv[++i];
    else if (strcmp(argv[i], "--framedump-skip") == 0 && i+1 < argc) dump_skip = atoi(argv[++i]);
    else if (strcmp(argv[i], "--framedump-max") == 0 && i+1 < argc) dump_max = atoi(argv[++i]);
  }

  /* Resolve and load ROM */
  char rom_path[4096];
  if (snesrecomp_launcher_resolve_rom(argc, argv, rom_path, sizeof(rom_path), 0) < 0) {
    fprintf(stderr, "No ROM path provided.\n");
    return 1;
  }
  size_t rom_size = 0;
  uint8 *rom_data = ReadWholeFile(rom_path, &rom_size);
  if (!rom_data) { fprintf(stderr, "Failed to read ROM: %s\n", rom_path); return 1; }
  fprintf(stderr, "Loaded ROM: %s (%zu bytes)\n", rom_path, rom_size);

  snesrecomp_anchor_to_exe_dir();

  /* Load config */
  Config_Load("config.ini");
  Input_Init("config.ini");
  BmpDump_Init(dump_dir, dump_skip, dump_max);

  /* Register game and initialize SNES */
  RtlRegisterGame(&kMrNutzGameInfo);
  g_snes = SnesInit(rom_data, (int)rom_size);
  if (!g_snes) { fprintf(stderr, "SnesInit failed\n"); return 1; }

  /* SPC player */
  g_spc_player = MrNutzSpcPlayer_Create();
  g_spc_player->initialize(g_spc_player);

  /* Music manager — MP3 replacement.
   * Loads mods/audio/music_map.ini. If present, MP3s replace SPC music.
   * If absent, original SPC music plays (no interference). */
  MusicManager_Init("mods/audio");

  /* SDL init */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD) < 0) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  /* Create window */
  if (!Window_Init(g_app_config.window_scale)) {
    fprintf(stderr, "Window_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  /* Audio */
  if (g_app_config.enable_audio) {
    if (!AudioOut_Init())
      fprintf(stderr, "[audio] failed: %s\n", SDL_GetError());
  }

  /* Gamepad */
  Input_OpenGamepad();

  /* Saves */
  MkDir("saves");
  RtlReadSram();

  fprintf(stderr, "Starting main loop...\n");
  fprintf(stderr, "Hotkeys: F=FPS, P=Pause, Tab=Turbo, F1=Save, F2=Load\n");

  /* Main loop */
  bool running = true;
  int frame_count = 0;
  uint64_t perf_freq = SDL_GetPerformanceFrequency();
  uint64_t fps_timer = SDL_GetPerformanceCounter();
  uint64_t frame_start;
  int fps_frames = 0;

  while (running) {
    frame_start = SDL_GetPerformanceCounter();

    /* Events */
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT: running = false; break;
        case SDL_EVENT_KEY_DOWN: Input_HandleEvent(SNESRECOMP_SDL_EVENT_KEY(event), true); break;
        case SDL_EVENT_KEY_UP: Input_HandleEvent(SNESRECOMP_SDL_EVENT_KEY(event), false); break;
      }
    }

    if (g_paused) { SDL_Delay(16); continue; }
    RtlAudioSetFastForward(g_turbo);

    /* Input + game frame */
    Input_Poll();
    RtlRunFrame(g_input_state);

    /* Music update (detect track changes) */
    MusicManager_Update();

    /* Render */
    if (!g_turbo || (frame_count & 1)) {
      MrNutzDrawPpuFrame();
      FPS_Render(g_my_pixels, (size_t)g_snes_width * 4, g_curr_fps);
      BmpDump_Capture(g_my_pixels, 256, g_snes_height, frame_count);
      Window_Present();
    }

    /* FPS counter */
    fps_frames++;
    uint64_t now = SDL_GetPerformanceCounter();
    if (now - fps_timer >= perf_freq) {
      g_curr_fps = (int)((uint64_t)fps_frames * perf_freq / (now - fps_timer));
      fps_frames = 0;
      fps_timer = now;
      char title[128];
      snprintf(title, sizeof(title), "Mr. Nutz (Recompiled)  [%d fps]", g_curr_fps);
      Window_SetTitle(title);
    }

    /* Frame pacer: target 60 fps */
    if (!g_turbo) {
      double elapsed = (double)(now - frame_start) * 1000.0 / perf_freq;
      if (elapsed < 16.0)
        SDL_Delay((uint32_t)(16.0 - elapsed));
    }

    frame_count++;
  }

  /* Cleanup */
  RtlWriteSram();
  MusicManager_Destroy();
  AudioOut_Destroy();
  Window_Destroy();
  SDL_Quit();
  free(rom_data);
  return 0;
}
