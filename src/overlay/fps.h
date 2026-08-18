#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"

/* Overlay: FPS counter rendered directly into the pixel buffer
 * using a 5x7 dot font. Toggle with the F key. */

void FPS_Render(uint8 *pixels, size_t pitch, int fps);
void FPS_Toggle(void);
bool FPS_IsEnabled(void);
void FPS_SetEnabled(bool enabled);
