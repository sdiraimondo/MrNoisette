#pragma once
#include <stdint.h>
#include "types.h"

/* Level module: Custom level data format.
 * Defines the structure of .mrlvl files (JSON-encoded).
 * Fields mirror the game's internal level data layout. */

#define LEVEL_TILE_W 32
#define LEVEL_TILE_H 27
#define LEVEL_PALETTE_SIZE 256

typedef struct {
  uint16 tile_id;       /* Tile index in the tileset */
  uint8  palette;       /* Palette number (0-7) */
  uint8  flags;          /* H-flip, V-flip, priority */
} LevelTile;

typedef struct {
  uint16 type;           /* Enemy/object type ID */
  uint16 x;              /* Pixel X position */
  uint16 y;              /* Pixel Y position */
  uint8  param1;         /* Object-specific parameter */
  uint8  param2;
} LevelObject;

typedef struct {
  char name[32];                 /* Level name */
  LevelTile tiles[LEVEL_TILE_H][LEVEL_TILE_W];  /* Background tilemap */
  LevelObject objects[64];       /* Object placements */
  uint16 palette[LEVEL_PALETTE_SIZE];  /* CGRAM palette entries */
  uint8  music_track_id;         /* Music track to play (maps to MP3) */
  uint16 bg_scroll_h;            /* Background horizontal scroll */
  uint16 bg_scroll_v;            /* Background vertical scroll */
  uint8  bg_mode;                /* PPU BG mode (1-7) */
  uint8  screen_flags;           /* TM register value */
} LevelData;
