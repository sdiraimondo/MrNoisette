#pragma once
#include "cpu_state.h"
#include "common_cpu_infra.h"

/* Public declarations for the Mr Nutz runtime library (RTL).
 * These functions are called by the framework via RtlGameInfo. */

extern const RtlGameInfo kMrNutzGameInfo;

/* Per-frame game logic driver. Called by the framework's RtlRunFrame
 * every frame. On the first call, runs the reset handler. On
 * subsequent calls, advances PPU to vblank, runs the NMI handler,
 * then resumes the main loop until WAI. */
void RunOneFrameOfGame(void);

/* PPU frame renderer with per-scanline HDMA processing. Registered as
 * the draw_ppu_frame callback in kMrNutzGameInfo. */
void MrNutzDrawPpuFrame(void);

/* Game state enumeration. Detected by reading game_mode_ptr and
 * nmi_vector from WRAM. Used by the host for state-dependent
 * behavior (blink, level injection, music switching, etc.). */
enum MrNutzGameState {
  MRNUTZ_STATE_BOOT = 0,
  MRNUTZ_STATE_TITLE,
  MRNUTZ_STATE_DEMO,
  MRNUTZ_STATE_PLAYING,
  MRNUTZ_STATE_PAUSED,
  MRNUTZ_STATE_LEVEL_LOAD,
};

/* Returns the current game state by inspecting WRAM variables.
 * Called by the host each frame after RtlRunFrame. */
enum MrNutzGameState MrNutz_GetGameState(void);
