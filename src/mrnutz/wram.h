#pragma once
#include "types.h"
#include "common_rtl.h"

/* Mr Nutz WRAM variable aliases. These map game-internal memory locations
 * to named symbols for readability. All addresses are in bank $7E (WRAM)
 * unless otherwise noted. */

/* Host-protocol frame counter. Bumped once per RtlRunFrame call. */
extern uint16 counter_global_frames;

/* Blink timer ($0570-$0571, 16-bit). Counts down from $F0 (240). When it
 * reaches 0, the main loop's $9DE198 routine sets the game mode pointer
 * to $D1D7 (the title screen blink handler). */
#define blink_timer_lo    (*(uint8*)(g_ram + 0x0570))
#define blink_timer_hi    (*(uint8*)(g_ram + 0x0571))

/* Blink fade counter ($056F, 8-bit). Decremented by the $D1D7 handler
 * every 2 frames. INIDISP is set to this value. When it reaches 0, the
 * handler forces blank ($80) and switches game mode to $CC08. */
#define blink_fade_counter (*(uint8*)(g_ram + 0x056F))

/* Blink sub-counter ($056E, 8-bit). Incremented by $80 each frame by the
 * $D1D7 handler. When it overflows (carry), blink_fade_counter is
 * decremented. */
#define blink_sub_counter  (*(uint8*)(g_ram + 0x056E))

/* Game mode pointer ($057E-$057F, 16-bit). Used by the game logic
 * dispatch at $8180E2: JMP ($057E). Values observed:
 *   $D0E8 = initial / fade-in mode
 *   $CC08 = title screen static (just RTS, does nothing)
 *   $D1D7 = title screen blink handler */
#define game_mode_ptr_lo   (*(uint8*)(g_ram + 0x057E))
#define game_mode_ptr_hi   (*(uint8*)(g_ram + 0x057F))

/* NMI entry vector ($01DC-$01DD, 16-bit). The NMI handler at $80:834D
 * does JMP ($01DC). Values observed:
 *   $8357 = initial NMI handler (JML to $9DB413)
 *   $835B = in-game NMI handler */
#define nmi_vector_lo      (*(uint8*)(g_ram + 0x01DC))
#define nmi_vector_hi      (*(uint8*)(g_ram + 0x01DD))

/* Frame processing vector ($01DE-$01DF, 16-bit). The NMI handler at
 * $9DB413 does JMP ($01DE) when V counter is $E0-$E2. Set to $E1B6
 * during initialization. */
#define frame_proc_vec_lo  (*(uint8*)(g_ram + 0x01DE))
#define frame_proc_vec_hi  (*(uint8*)(g_ram + 0x01DF))

/* INIDISP shadow ($1A7F, 8-bit). The NMI handler's frame processing
 * copies this to $2100 (INIDISP) every frame. $0F = full brightness,
 * $80 = forced blank, $00 = black. */
#define inidisp_shadow     (*(uint8*)(g_ram + 0x1A7F))

/* NMI vblank flag ($0B9D, 8-bit). The game's main loop spin-waits on
 * this location. */
#define waiting_for_vblank (*(uint8*)(g_ram + 0x0B9D))

/* Frame counter ($35, 8-bit, direct page). Incremented by the frame
 * processing routine. The main loop uses CMP $35 to detect frame
 * changes. */
#define frame_counter_dp   (*(uint8*)(g_ram + 0x0035))

/* Second game mode dispatch pointer ($058C-$058D, 16-bit). Used by the
 * second dispatch at $8180EB: JMP ($058C). */
#define game_mode2_ptr_lo  (*(uint8*)(g_ram + 0x058C))
#define game_mode2_ptr_hi  (*(uint8*)(g_ram + 0x058D))

/* Transition timer ($1840, 16-bit). Used by the frame processing code
 * after the controller check. When it reaches 0, the game starts the
 * demo or transitions to the next mode. */
#define transition_timer   (*(uint16*)(g_ram + 0x1840))

/* Music track ID ($00EC, 8-bit). The game stores the current music
 * track here. The SPC command sent to $2140 is: 0x03 | track_id.
 * Bit 7 is toggled as a handshake after each command. The actual
 * track ID is the value with bit 7 cleared (track_id & 0x7F). */
#define music_track_id     (*(uint8*)(g_ram + 0x00EC))
