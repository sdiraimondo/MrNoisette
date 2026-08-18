#include "mrnutz/mrnutz_spc_player.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "snes/spc.h"
#include "snes/dsp_regs.h"

typedef struct MrNutzSpcPlayer {
  SpcPlayer base;
  uint8 ram[65536];
} MrNutzSpcPlayer;

static void Dsp_Write(MrNutzSpcPlayer *p, uint8_t reg, uint8 value) {
  if (p->base.dsp)
    dsp_write(p->base.dsp, reg, value);
}

static const uint8 kDefDspRegs[12] = {
  MVOLL, MVOLR, EVOLL, EVOLR, FLG, EFB, PMON, NON, EON, DIR, ESA, EDL
};
static const uint8 kDefDspValues[12] = {
  0x7F, 0x7F, 0, 0, 0x2F, 0x60, 0, 0, 0, 0x80, 0x60, 2
};

static void Spc_Reset(MrNutzSpcPlayer *p) {
  memset(p->ram, 0, 0x500);
  memset(p->base.input_ports, 0, sizeof(p->base.input_ports));
  for (int i = 11; i >= 0; i--)
    Dsp_Write(p, kDefDspRegs[i], kDefDspValues[i]);
}

static void MrNutzSpcPlayer_Initialize(SpcPlayer *p_in) {
  MrNutzSpcPlayer *p = (MrNutzSpcPlayer *)p_in;
  dsp_reset(p->base.dsp);
  Spc_Reset(p);
}

static int s_upload_count = 0;

static void MrNutzSpcPlayer_Upload(SpcPlayer *p_in, const uint8_t *data) {
  MrNutzSpcPlayer *p = (MrNutzSpcPlayer *)p_in;
  s_upload_count++;

  /* Log SPC uploads: the game uploads different SPC data per level.
   * Each upload contains the SPC driver + song data. By counting
   * uploads and logging the target addresses, we can identify
   * which song corresponds to which level. */
  int total_size = 0;
  int block_count = 0;
  const uint8_t *d = data;
  int first_target = -1;
  while (1) {
    int nb = *(uint16 *)(d);
    if (nb == 0) break;
    int tg = *(uint16 *)(d + 2);
    if (first_target < 0) first_target = tg;
    total_size += nb;
    block_count++;
    d += 4 + nb;
  }
  fprintf(stderr, "[spc] upload #%d: %d blocks, %d bytes, first_target=$%04X\n",
          s_upload_count, block_count, total_size, first_target);

  Dsp_Write(p, FLG, 0x60);
  Dsp_Write(p, KOF, 0xff);
  for (;;) {
    int numbytes = *(uint16 *)(data);
    if (numbytes == 0) break;
    int target = *(uint16 *)(data + 2);
    data += 4;
    do {
      p->ram[target++ & 0xffff] = *data++;
    } while (--numbytes);
  }
  p->base.port_to_snes[0] = p->base.port_to_snes[1] =
    p->base.port_to_snes[2] = p->base.port_to_snes[3] = 0;
  memset(p->base.input_ports, 0, sizeof(p->base.input_ports));
  Dsp_Write(p, FLG, 0x20);
}

SpcPlayer *MrNutzSpcPlayer_Create(void) {
  MrNutzSpcPlayer *p = (MrNutzSpcPlayer *)malloc(sizeof(MrNutzSpcPlayer));
  memset(p, 0, sizeof(MrNutzSpcPlayer));
  p->base.dsp = dsp_init(p->ram);
  p->base.initialize = &MrNutzSpcPlayer_Initialize;
  p->base.upload = &MrNutzSpcPlayer_Upload;
  return &p->base;
}
