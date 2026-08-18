#pragma once
#include <stdint.h>
#include <stdbool.h>

/* Platform abstraction: config.ini parser.
 * Parses [General], [Graphics], [Sound], [Keyboard], [Gamepad],
 * [Music], and [Levels] sections. */

typedef struct {
  int window_scale;
  bool disable_frame_delay;
  bool skip_launcher;
  bool fullscreen;
  bool enable_audio;
  int audio_freq;
  int audio_channels;
  int audio_samples;
  /* Music settings */
  int music_mode;   /* 0=SPC only, 1=MP3 only, 2=mixed */
  int music_volume; /* 0-100 */
  int sfx_volume;   /* 0-100 */
} AppConfig;

extern AppConfig g_app_config;

/* Loads config.ini. Falls back to defaults if file is missing. */
void Config_Load(const char *filename);
