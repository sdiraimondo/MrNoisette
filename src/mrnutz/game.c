#include "cpu_state.h"
#include "common_cpu_infra.h"
#include "mrnutz/game.h"
#include "mrnutz/wram.h"

const RtlGameInfo kMrNutzGameInfo = {
  .title = "mrnutz",
  .initialize = NULL,
  .run_frame = &RunOneFrameOfGame,
  .draw_ppu_frame = &MrNutzDrawPpuFrame,
  .save_name_prefix = "save",
  .state_save_extra = NULL,
  .state_load_extra = NULL,
  .on_state_loaded = NULL,
  .session_reset = NULL,
};

enum MrNutzGameState MrNutz_GetGameState(void) {
  uint16 mode = game_mode_ptr_lo | (game_mode_ptr_hi << 8);
  uint16 nmi = nmi_vector_lo | (nmi_vector_hi << 8);

  if (mode == 0xD0E8) return MRNUTZ_STATE_BOOT;
  if (mode == 0xCC08 && nmi == 0x8357) return MRNUTZ_STATE_TITLE;
  if (mode == 0xD1D7) return MRNUTZ_STATE_TITLE;
  if (nmi == 0x835B) return MRNUTZ_STATE_PLAYING;
  if (inidisp_shadow & 0x80) return MRNUTZ_STATE_LEVEL_LOAD;
  return MRNUTZ_STATE_DEMO;
}
