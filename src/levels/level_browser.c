#include "levels/level_browser.h"
#include <string.h>

/* Stub implementation. The level browser will use the same 5x7
 * pixel font as the FPS overlay to render a scrollable list of
 * .mrlvl filenames over the PPU output. */

static bool s_visible = false;
static const char *s_dir = NULL;

void LevelBrowser_Show(const char *dir) { s_dir = dir; s_visible = true; }
bool LevelBrowser_IsVisible(void) { return s_visible; }
void LevelBrowser_HandleInput(uint32 input_state) { (void)input_state; }
void LevelBrowser_Render(uint8 *pixels, size_t pitch, int height) { (void)pixels; (void)pitch; (void)height; }
void LevelBrowser_Hide(void) { s_visible = false; s_dir = NULL; }
const char *LevelBrowser_GetSelection(void) { return NULL; }
