#pragma once
#include "types.h"
#include <stdint.h>
#include <stdbool.h>

/* Platform abstraction: SDL window, renderer, and texture management. */

/* Pixel buffer (256x240 ARGB, shared with PPU renderer). */
extern uint8 g_my_pixels[256 * 4 * 240];
extern int g_snes_width;
extern int g_snes_height;

/* Initializes SDL video, creates window, renderer, and texture.
 * Uses SDL_PIXELFORMAT_XRGB8888 to handle PPU's alpha=0 pixels.
 * Vsync is disabled; frame pacing is handled by the main loop. */
bool Window_Init(int scale);
void Window_Destroy(void);

/* Renders the pixel buffer to the screen. The caller is responsible
 * for filling g_my_pixels before calling this. */
void Window_Present(void);
void Window_SetTitle(const char *title);
void Window_Destroy(void);
