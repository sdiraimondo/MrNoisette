#include "levels/level_reader.h"
#include <stdio.h>
#include <string.h>

/* Stub implementation. JSON parsing will use cJSON (single-file lib). */

bool LevelReader_Load(const char *path, LevelData *out) {
  (void)path;
  memset(out, 0, sizeof(LevelData));
  return false; /* Not yet implemented */
}

int LevelReader_ListDir(const char *dir, char filenames[][64], int max_files) {
  (void)dir;
  (void)filenames;
  (void)max_files;
  return 0; /* Not yet implemented */
}
