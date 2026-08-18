#include "levels/level_injector.h"
#include <string.h>

/* Stub implementation. Level injection requires understanding the
 * game's DMA-to-VRAM level load sequence, which runs on the
 * interpreter. The injector will monitor hardware register writes
 * to detect when the game starts loading level data. */

static const LevelData *s_level = NULL;
static bool s_active = false;

void LevelInjector_SetLevel(const LevelData *level) {
  s_level = level;
  s_active = (level != NULL);
}

void LevelInjector_Update(void) {
  /* TODO: Monitor $420B (MDMAEN) for DMA enable, then intercept
   * $2116 (VMADDL/H) and $2118 (VMDATAIN) writes to substitute
   * custom tilemap data. Also monitor $2121 (CGADD) and $2122
   * (CGDATA) for palette injection. */
  (void)0;
}

bool LevelInjector_IsActive(void) { return s_active; }
