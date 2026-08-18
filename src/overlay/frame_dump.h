#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "types.h"

/* Overlay: BMP frame capture for screenshots and demo recording.
 * Activated via --framedump command-line argument. */

void BmpDump_Init(const char *dir, int skip, int max);
void BmpDump_Capture(uint8 *pixels, int w, int h, int frame_num);
bool BmpDump_IsActive(void);
