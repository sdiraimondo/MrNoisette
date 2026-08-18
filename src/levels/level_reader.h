#pragma once
#include "levels/level_format.h"

/* Level module: Reader for .mrlvl custom level files.
 * Parses JSON level data and populates a LevelData struct. */

/* Loads a .mrlvl file. Returns true on success. */
bool LevelReader_Load(const char *path, LevelData *out);

/* Lists all .mrlvl files in a directory. Returns the count.
 * Fills filenames[] with up to max_files entries. */
int LevelReader_ListDir(const char *dir, char filenames[][64], int max_files);
