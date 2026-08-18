#pragma once
#include <stdint.h>
#include <stdbool.h>

/* Audio module: Music manager.
 * Detects the current in-game music track by monitoring WRAM $00EC
 * (the game's music track variable). When the track changes, looks
 * up the corresponding MP3 file in music_map.ini and plays it.
 *
 * If no MP3 file exists for a track, or music_map.ini is missing,
 * the original SPC music from the ROM plays normally (no interference).
 *
 * music_map.ini format (in mods/audio/):
 *   [Tracks]
 *   0 = Title.mp3
 *   1 = Woody Land 1 & 2.mp3
 *   ...
 *
 * The MP3 files are loaded from mods/audio/ (relative to the exe).
 */

/* Initializes the music manager. Loads music_map.ini from the given
 * directory (e.g., "mods/audio"). Returns true if any tracks were
 * mapped. */
bool MusicManager_Init(const char *audio_dir);

/* Called every frame after RtlRunFrame. Monitors WRAM $00EC for
 * track changes and triggers MP3 playback. */
void MusicManager_Update(void);

/* Shuts down the music manager and frees resources. */
void MusicManager_Destroy(void);

/* Returns the current track ID (0-127), or -1 if none. */
int MusicManager_GetCurrentTrack(void);

/* Returns true if an MP3 is currently playing. */
bool MusicManager_IsMP3Playing(void);
