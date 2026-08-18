#include "audio/audio_mixer.h"
#include <string.h>

static int s_music_mode = 0; /* 0=SPC only, 1=MP3 only, 2=mixed */
static float s_music_vol = 0.8f;
static float s_sfx_vol = 1.0f;

void AudioMixer_Init(int music_mode, int music_volume, int sfx_volume) {
  s_music_mode = music_mode;
  s_music_vol = (float)music_volume / 100.0f;
  s_sfx_vol = (float)sfx_volume / 100.0f;
}

void AudioMixer_Mix(const int16_t *spc_buf, const int16_t *mp3_buf,
                    int16_t *out, int samples) {
  if (s_music_mode == 0 || !mp3_buf) {
    /* SPC only */
    for (int i = 0; i < samples * 2; i++)
      out[i] = (int16_t)(spc_buf[i] * s_sfx_vol);
  } else if (s_music_mode == 1) {
    /* MP3 only */
    for (int i = 0; i < samples * 2; i++)
      out[i] = (int16_t)(mp3_buf[i] * s_music_vol);
  } else {
    /* Mixed: SPC (SFX) + MP3 (music) */
    for (int i = 0; i < samples * 2; i++) {
      int32_t mix = (int32_t)(spc_buf[i] * s_sfx_vol + mp3_buf[i] * s_music_vol);
      if (mix > 32767) mix = 32767;
      if (mix < -32768) mix = -32768;
      out[i] = (int16_t)mix;
    }
  }
}
