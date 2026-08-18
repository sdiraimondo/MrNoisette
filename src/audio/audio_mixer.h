#pragma once
#include <stdint.h>

/* Audio module: Audio mixer.
 * Mixes SPC audio (sound effects) with MP3 audio (music) into a
 * single output stream. Configurable via config.ini [Music] section:
 *   MusicMode = SPC-only | MP3-only | Mixed
 */

void AudioMixer_Init(int music_mode, int music_volume, int sfx_volume);

/* Mixes SPC and MP3 samples into the output buffer.
 * spc_buf: SPC-rendered samples (SFX)
 * mp3_buf: MP3-decoded samples (music), or NULL if no MP3 playing
 * out: mixed output buffer
 * samples: number of stereo samples to mix */
void AudioMixer_Mix(const int16_t *spc_buf, const int16_t *mp3_buf,
                    int16_t *out, int samples);
