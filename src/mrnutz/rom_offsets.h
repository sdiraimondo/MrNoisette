#pragma once

/* ROM constants for Mr Nutz (USA) (En,Fr).
 * Extracted from ROM analysis and tier2 log observations. */

/* SNES interrupt vectors (native mode, at $00:FFEA/$FFEE/$FFFC). */
#define ROM_RESET_VECTOR  0x0080B5
#define ROM_NMI_VECTOR    0x008349
#define ROM_IRQ_VECTOR    0x0083B3

/* NMI handler continuation ($00:8349 does JML $80:834D). */
#define NMI_HANDLER_ADDR  0x80834D

/* Game mode handler addresses (used as $057E values). */
#define GAME_MODE_FADE_IN   0xD0E8  /* Initial fade-in, frames 8-83 */
#define GAME_MODE_TITLE     0xCC08  /* Title screen static (RTS, no-op) */
#define GAME_MODE_BLINK     0xD1D7  /* Title screen blink handler */

/* NMI entry vector values (used as $01DC values). */
#define NMI_ENTRY_INITIAL  0x8357  /* JML $9DB413 */
#define NMI_ENTRY_INGAME   0x835B  /* In-game NMI handler */

/* ROM info. */
#define ROM_SHA256 \
  "3472dd574b50aed2fa998f464398db4fbb00f5a300a672c3737ee9336a008a16"
#define ROM_SIZE 1048576
#define ROM_MAPPING "lorom"
