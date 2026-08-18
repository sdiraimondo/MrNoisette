#include "audio/music_manager.h"
#include "audio/mp3_player.h"
#include "mrnutz/wram.h"
#include "common_rtl.h"
#include "snes/snes.h"
#include "mrnutz/game.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX_TRACKS 128
#define MAX_PATH 512

static Mp3Player *s_player = NULL;
static char s_audio_dir[MAX_PATH];
static char s_track_files[MAX_TRACKS][MAX_PATH];
static int s_current_track = -1;
static bool s_map_loaded = false;

/* Previous NMI vector value — detects title→gameplay transition */
static uint8_t s_prev_nmi_vector = 0;

/* Previous game state */
static enum MrNutzGameState s_prev_state = MRNUTZ_STATE_BOOT;

static bool file_exists(const char *path) {
  struct stat st;
  return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static void trim(char *s) {
  char *p = s + strlen(s) - 1;
  while (p >= s && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
    *p-- = '\0';
}

bool MusicManager_Init(const char *audio_dir) {
  strncpy(s_audio_dir, audio_dir, sizeof(s_audio_dir)-1);
  s_audio_dir[sizeof(s_audio_dir)-1] = '\0';
  s_player = Mp3Player_Create();
  s_current_track = -1;
  s_map_loaded = false;
  s_prev_nmi_vector = 0;
  s_prev_state = MRNUTZ_STATE_BOOT;

  char map_path[MAX_PATH];
  snprintf(map_path, sizeof(map_path), "%s/music_map.ini", s_audio_dir);
  FILE *f = fopen(map_path, "r");
  if (!f) {
    fprintf(stderr, "[music] No music_map.ini at %s — SPC fallback only\n", map_path);
    return false;
  }

  char line[256];
  bool in_tracks = false;
  int mapped = 0;
  while (fgets(line, sizeof(line), f)) {
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;
    if (*p == '[') {
      in_tracks = (strncmp(p, "[Tracks]", 8) == 0);
      continue;
    }
    if (!in_tracks) continue;

    char *eq = strchr(p, '=');
    if (!eq) continue;
    *eq = '\0';
    char *val = eq + 1;
    while (*val == ' ') val++;
    trim(p);
    trim(val);

    int id = atoi(p);
    if (id < 0 || id >= MAX_TRACKS) continue;

    strncpy(s_track_files[id], val, MAX_PATH-1);
    s_track_files[id][MAX_PATH-1] = '\0';

    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s/%s", s_audio_dir, s_track_files[id]);
    if (file_exists(full_path)) {
      mapped++;
      fprintf(stderr, "[music] track %d → %s ✓\n", id, s_track_files[id]);
    } else {
      s_track_files[id][0] = '\0';
    }
  }
  fclose(f);

  s_map_loaded = (mapped > 0);
  fprintf(stderr, "[music] %d tracks mapped from %s\n", mapped, map_path);
  return s_map_loaded;
}

/* Play the MP3 for a given track ID, or stop if no MP3 exists */
static void play_track(int track) {
  if (track == s_current_track) return;
  s_current_track = track;

  if (track < 0 || track >= MAX_TRACKS || s_track_files[track][0] == '\0') {
    if (Mp3Player_IsPlaying(s_player)) {
      fprintf(stderr, "[music] track %d: no MP3, SPC fallback\n", track);
      Mp3Player_Stop(s_player);
    }
    return;
  }

  char full_path[MAX_PATH];
  snprintf(full_path, sizeof(full_path), "%s/%s", s_audio_dir, s_track_files[track]);
  fprintf(stderr, "[music] track %d: playing %s\n", track, s_track_files[track]);
  Mp3Player_Play(s_player, full_path, true);
}

void MusicManager_Update(void) {
  if (!s_map_loaded || !s_player) return;

  enum MrNutzGameState state = MrNutz_GetGameState();
  uint8_t nmi_vec = nmi_vector_lo;

  /* Detect state transitions */
  if (state != s_prev_state || nmi_vec != s_prev_nmi_vector) {
    fprintf(stderr, "[music] state %d→%d, NMI $%02X→$%02X\n",
            s_prev_state, state, s_prev_nmi_vector, nmi_vec);

    switch (state) {
      case MRNUTZ_STATE_TITLE:
        play_track(0); /* Title.mp3 */
        break;
      case MRNUTZ_STATE_PLAYING:
        play_track(1); /* Woody Land 1 & 2.mp3 (first level) */
        break;
      default:
        break;
    }

    s_prev_state = state;
    s_prev_nmi_vector = nmi_vec;
  }
}

void MusicManager_Destroy(void) {
  Mp3Player_Destroy(s_player);
  s_player = NULL;
  s_current_track = -1;
}

int MusicManager_GetCurrentTrack(void) { return s_current_track; }
bool MusicManager_IsMP3Playing(void) { return s_player && Mp3Player_IsPlaying(s_player); }
