#include "snes/ppu.h"
#include "snes/dma.h"
#include "mrnutz/ppu_render.h"
#include "mrnutz/wram.h"

extern uint8 g_my_pixels[];
extern int g_snes_height;
extern uint8 g_snesrecomp_last_hdmaen;

void MrNutzDrawPpuFrame(void) {
  int height = g_snes_height;
  if (height <= 0) height = 224;
  PpuBeginDrawing(g_ppu, g_my_pixels, (size_t)256 * 4, 0);

  /* Process HDMA between scanlines (needed for per-line effects like
   * gradients, windowing, color math, and the title screen cursor blink). */
  SimpleHdma hdma_chans[3];
  Dma *dma = g_dma;
  dma_startDma(dma, g_snesrecomp_last_hdmaen, true);
  SimpleHdma_Init(&hdma_chans[0], &dma->channel[5]);
  SimpleHdma_Init(&hdma_chans[1], &dma->channel[6]);
  SimpleHdma_Init(&hdma_chans[2], &dma->channel[7]);

  for (int i = 0; i <= height; i++) {
    ppu_runLine(g_ppu, i);
    SimpleHdma_DoLine(&hdma_chans[0]);
    SimpleHdma_DoLine(&hdma_chans[1]);
    SimpleHdma_DoLine(&hdma_chans[2]);
  }
}
