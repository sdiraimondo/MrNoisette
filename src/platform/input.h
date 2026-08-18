#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "desktop/sdl_compat.h"
#include "types.h"

/* Platform abstraction: input system.
 * Polls keyboard and gamepad state every frame. Both are active
 * simultaneously (OR-ed). Uses SDL_GetKeyboardState for keyboard
 * (polling, not events) to avoid missed presses on slow frames. */

/* Input state bitfield (12-bit SNES controller word + Start workaround). */
extern uint32 g_input_state;

/* Initializes the input system with config.ini bindings. */
bool Input_Init(const char *config_file);

/* Polls keyboard and gamepad, updates g_input_state. */
void Input_Poll(void);

/* Handles hotkey events (P, Tab, F, F1, F2). Called from the event loop. */
void Input_HandleEvent(int key, bool pressed);

/* Opens the first connected gamepad. */
void Input_OpenGamepad(void);

/* Global flags set by hotkeys. */
extern bool g_paused;
extern bool g_turbo;
