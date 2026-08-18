#pragma once
#include "levels/level_format.h"

/* Level module: In-game level browser overlay.
 * Renders a simple text menu over the PPU output, listing
 * available .mrlvl files from the assets/levels/ directory.
 * The player can navigate with Up/Down and select with Start. */

/* Shows the level browser overlay. Called when the player presses
 * a designated key on the title screen. */
void LevelBrowser_Show(const char *dir);

/* Returns true if the browser is currently visible. */
bool LevelBrowser_IsVisible(void);

/* Handles input for the browser (Up/Down/Start). */
void LevelBrowser_HandleInput(uint32 input_state);

/* Renders the browser overlay onto the pixel buffer. */
void LevelBrowser_Render(uint8 *pixels, size_t pitch, int height);

/* Hides the browser. */
void LevelBrowser_Hide(void);

/* Returns the selected level file path, or NULL if none selected. */
const char *LevelBrowser_GetSelection(void);
