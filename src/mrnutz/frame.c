#include <stdint.h>
#include "cpu_state.h"
#include "common_rtl.h"
#include "common_cpu_infra.h"
#include "snes/snes.h"
#include "snes/ppu.h"
#include "snes/interp_bridge.h"
#include "mrnutz/frame.h"
#include "mrnutz/wram.h"
#include "mrnutz/recomp_decls.h"
#include "mrnutz/rom_offsets.h"

extern uint8 g_my_pixels[];
extern int g_snes_height;

uint16 counter_global_frames = 0;

static bool g_did_reset = false;
static uint32_t g_resume_pc = 0;

void RunOneFrameOfGame(void) {
  if (!g_did_reset) {
    cpu_state_init(&g_cpu, g_ram);
    g_snes->inNmi = true;
    interp_bridge_run_until_quiescent(&g_cpu, ROM_RESET_VECTOR);
    g_resume_pc = interp_bridge_lle_resume_pc();
    if (g_resume_pc == 0) g_resume_pc = ROM_RESET_VECTOR;
    g_did_reset = true;
    return;
  }

  /* Advance the PPU beam to vblank (scanline 225, h=0) so the NMI
   * handler's reads of $2137/$213D (SLHV/OPHCT/OPVCT) return the
   * correct H/V counter values it expects at NMI entry time. */
  snes_advance_master_cycles(g_snes, 1364u * 225u - g_snes->hPos);
  g_snes->inVblank = true;
  g_snes->inNmi = true;

  cpu_push_interrupt_frame_at(&g_cpu, g_resume_pc);
  interp_bridge_run_interrupt(&g_cpu, ROM_NMI_VECTOR);
  interp_bridge_run_until_quiescent(&g_cpu, g_resume_pc);
  g_resume_pc = interp_bridge_lle_resume_pc();
  if (g_resume_pc == 0) g_resume_pc = ROM_RESET_VECTOR;
  counter_global_frames++;
}
