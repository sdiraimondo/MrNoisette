#pragma once
#include <stdint.h>
#include <stdbool.h>

/* Audio module: MP3 playback engine.
 * Manages playback state (play/stop/loop/fade), buffer management,
 * and volume control for a single MP3 stream. */

typedef struct Mp3Player Mp3Player;

Mp3Player *Mp3Player_Create(void);
void Mp3Player_Destroy(Mp3Player *p);

/* Loads and starts playing an MP3 file. If loop is true, the track
 * restarts when it reaches the end. */
bool Mp3Player_Play(Mp3Player *p, const char *path, bool loop);

/* Stops playback. */
void Mp3Player_Stop(Mp3Player *p);

/* Reads decoded PCM samples into the output buffer.
 * Returns the number of samples actually decoded. */
int Mp3Player_Read(Mp3Player *p, int16_t *out, int max_samples);

/* Sets the playback volume (0-100). */
void Mp3Player_SetVolume(Mp3Player *p, int volume);

/* Returns true if the player is currently playing. */
bool Mp3Player_IsPlaying(Mp3Player *p);
