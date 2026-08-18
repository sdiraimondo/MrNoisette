#include "audio/mp3_player.h"
#include "audio/mp3_decoder.h"
#include <stdlib.h>

/* Stub implementation. Will be completed with minimp3 + playback buffer. */

struct Mp3Player {
  Mp3Decoder *dec;
  bool playing;
  bool loop;
  int volume;
};

Mp3Player *Mp3Player_Create(void) {
  Mp3Player *p = calloc(1, sizeof(Mp3Player));
  p->dec = Mp3Decoder_Create();
  p->volume = 80;
  return p;
}

void Mp3Player_Destroy(Mp3Player *p) {
  if (!p) return;
  Mp3Decoder_Destroy(p->dec);
  free(p);
}

bool Mp3Player_Play(Mp3Player *p, const char *path, bool loop) {
  p->loop = loop;
  p->playing = Mp3Decoder_OpenFile(p->dec, path);
  return p->playing;
}

void Mp3Player_Stop(Mp3Player *p) { p->playing = false; }

int Mp3Player_Read(Mp3Player *p, int16_t *out, int max) {
  if (!p->playing) return 0;
  int n = Mp3Decoder_Read(p->dec, out, max);
  if (n == 0 && p->loop) {
    Mp3Decoder_Rewind(p->dec);
    n = Mp3Decoder_Read(p->dec, out, max);
  }
  if (n == 0) p->playing = false;
  /* Apply volume scaling */
  if (p->volume != 100) {
    float scale = p->volume / 100.0f;
    for (int i = 0; i < n; i++) out[i] = (int16_t)(out[i] * scale);
  }
  return n;
}

void Mp3Player_SetVolume(Mp3Player *p, int vol) {
  if (vol < 0) vol = 0; if (vol > 100) vol = 100;
  p->volume = vol;
}

bool Mp3Player_IsPlaying(Mp3Player *p) { return p && p->playing; }
