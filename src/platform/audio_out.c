#include "platform/audio_out.h"
#include "common_rtl.h"
#include "desktop/sdl_compat.h"

static SDL_AudioDeviceID g_audio_device;
static SDL_mutex *g_audio_mutex;
static uint8 *g_audiobuffer, *g_audiobuffer_cur, *g_audiobuffer_end;
static int g_frames_per_block;
static uint8 g_audio_channels = 2;

#if SNESRECOMP_SDL3
static SDL_AudioStream *g_audio_stream;
static uint8 *g_audio_stream_buffer;
static size_t g_audio_stream_buffer_size;
#endif

void RtlApuLock(void) { SDL_LockMutex(g_audio_mutex); }
void RtlApuUnlock(void) { SDL_UnlockMutex(g_audio_mutex); }

static void FillAudioBuffer(Uint8 *stream, int len) {
  while (len > 0) {
    if (g_audiobuffer_cur >= g_audiobuffer_end) {
      RtlApuLock();
      RtlRenderAudio((int16_t *)g_audiobuffer, g_frames_per_block, g_audio_channels);
      RtlApuUnlock();
      g_audiobuffer_cur = g_audiobuffer;
    }
    int avail = (int)(g_audiobuffer_end - g_audiobuffer_cur);
    int to_copy = len < avail ? len : avail;
    memcpy(stream, g_audiobuffer_cur, to_copy);
    g_audiobuffer_cur += to_copy;
    stream += to_copy;
    len -= to_copy;
  }
}

#if SNESRECOMP_SDL3
void SDLCALL AudioOut_StreamCallback(void *ud, SDL_AudioStream *stream,
                                      int additional_amount, int total_amount) {
  (void)ud; (void)total_amount;
  if (additional_amount <= 0) return;
  if ((size_t)additional_amount > g_audio_stream_buffer_size) {
    g_audio_stream_buffer = realloc(g_audio_stream_buffer, additional_amount);
    g_audio_stream_buffer_size = additional_amount;
  }
  FillAudioBuffer(g_audio_stream_buffer, additional_amount);
  SDL_PutAudioStreamData(stream, g_audio_stream_buffer, additional_amount);
}
#endif

bool AudioOut_Init(void) {
  int freq = 44100;

#if SNESRECOMP_SDL3
  SDL_AudioSpec desired = {0};
  desired.freq = freq;
  desired.format = SDL_AUDIO_S16;
  desired.channels = g_audio_channels;
  g_audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                              &desired, AudioOut_StreamCallback, NULL);
  if (!g_audio_stream) return false;
  g_audio_device = SDL_GetAudioStreamDevice(g_audio_stream);
#else
  SDL_AudioSpec desired = {0}, obtained;
  desired.freq = freq;
  desired.format = AUDIO_S16SYS;
  desired.channels = g_audio_channels;
  desired.samples = 512;
  desired.callback = NULL; /* handled differently for SDL2 */
  g_audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
  if (!g_audio_device) return false;
  freq = obtained.freq;
#endif

  g_frames_per_block = (534 * freq + 16020) / 32040;
  size_t buf_size = (size_t)g_frames_per_block * g_audio_channels * 2;
  g_audiobuffer = malloc(buf_size);
  g_audiobuffer_cur = g_audiobuffer;
  g_audiobuffer_end = g_audiobuffer + buf_size;
  g_audio_mutex = SDL_CreateMutex();
  RtlSetAudioOutputRate(freq);
#if SNESRECOMP_SDL3
  SDL_ResumeAudioDevice(g_audio_device);
#else
  SDL_PauseAudioDevice(g_audio_device, 0);
#endif
  fprintf(stderr, "[audio] opened freq=%d block=%d\n", freq, g_frames_per_block);
  return true;
}

void AudioOut_Destroy(void) {
#if SNESRECOMP_SDL3
  if (g_audio_stream) SDL_DestroyAudioStream(g_audio_stream);
#else
  if (g_audio_device) SDL_CloseAudioDevice(g_audio_device);
#endif
  if (g_audio_mutex) SDL_DestroyMutex(g_audio_mutex);
  free(g_audiobuffer);
  free(g_audio_stream_buffer);
}

void AudioOut_Pause(bool paused) {
#if SNESRECOMP_SDL3
  /* SDL3: pause/resume via device */
#else
  if (paused) SDL_PauseAudioDevice(g_audio_device, 1);
  else SDL_PauseAudioDevice(g_audio_device, 0);
#endif
}
