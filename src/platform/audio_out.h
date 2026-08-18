#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "desktop/sdl_compat.h"

/* Platform abstraction: SDL audio output.
 * Provides a ring-buffer audio system that feeds SPC-rendered samples
 * to the SDL audio device. */

/* Initializes the SDL audio device at 44100 Hz, 2 channels.
 * Sets up the ring buffer and mutex for thread-safe access. */
bool AudioOut_Init(void);

/* Fills the audio stream with samples from the ring buffer.
 * Called by the SDL audio callback. */
void AudioOut_FillBuffer(uint8_t *stream, int len);

/* Locks/unlocks the audio mutex for SPC access. */
void AudioOut_Lock(void);
void AudioOut_Unlock(void);

/* Sets the SDL audio output rate for SPC rate conversion. */
void AudioOut_SetRate(int freq);

/* Pauses/resumes audio playback. */
void AudioOut_Pause(bool paused);

/* Shuts down the audio system. */
void AudioOut_Destroy(void);

/* SDL audio callback (SDL3 stream variant). */
#if SNESRECOMP_SDL3
void SDLCALL AudioOut_StreamCallback(void *ud, SDL_AudioStream *s, int add, int total);
#endif
