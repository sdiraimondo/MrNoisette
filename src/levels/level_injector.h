#pragma once
#include "levels/level_format.h"

/* Level module: Level injector.
 * Hooks into the game's level load sequence and substitutes custom
 * level data (tiles, palette, objects) for the original ROM data.
 *
 * Detection: monitors $420B (MDMAEN) and $2118 (VMDATAIN) for the
 * DMA burst that loads level tiles into VRAM. When detected,
 * substitutes custom tilemap data.
 *
 * Palette injection: writes custom CGRAM entries during the game's
 * palette load sequence.
 *
 * Object injection: writes custom object positions to WRAM at the
 * game's object list addresses. */

/* Activates level injection for the next level load. */
void LevelInjector_SetLevel(const LevelData *level);

/* Called each frame. Detects level load sequences and injects data. */
void LevelInjector_Update(void);

/* Returns true if a custom level is currently being injected. */
bool LevelInjector_IsActive(void);
