#pragma once

/* Mr Nutz control mapping constants.
 *
 * The snesrecomp framework applies SwapInputBits() which reverses all
 * 16 bits of the controller value before exposing it in $4218/$4219.
 * Each SNES button maps to a unique bit in the 12-bit input word.
 *
 * After SwapInputBits, the bits land at:
 *   B(7)→$4219.0  Y(6)→$4219.1  Select(5)→$4219.2  Start(4)→$4219.3
 *   Up(3)→$4219.4 Down(2)→$4219.5 Left(1)→$4219.6 Right(0)→$4219.7
 *   A(8)→$4218.7  X(9)→$4218.6   L(10)→$4218.5     R(11)→$4218.4
 *
 * Mr Nutz game-specific actions (discovered through testing):
 *   B       → move right
 *   Y       → move left
 *   A       → jump
 *   Select  → crouch
 *   R       → pause
 *   Start   → menu start (needs bit 11 workaround, see SN_START_WORKAROUND)
 */

/* Button bit values in the framework's 12-bit input format. */
enum {
  SN_B      = 0x080, /* bit 7  → $4219 bit 0 → game: move right */
  SN_Y      = 0x040, /* bit 6  → $4219 bit 1 → game: move left  */
  SN_A      = 0x100, /* bit 8  → $4218 bit 7 → game: jump       */
  SN_X      = 0x200, /* bit 9  → $4218 bit 6 → SNES X            */
  SN_L      = 0x400, /* bit 10 → $4218 bit 5 → SNES L            */
  SN_R      = 0x800, /* bit 11 → $4218 bit 4 → game: pause       */
  SN_SELECT = 0x020, /* bit 5  → $4219 bit 2 → game: crouch      */
  SN_START  = 0x010, /* bit 4  → $4219 bit 3 → game: menu start  */
  SN_UP     = 0x008, /* bit 3  → $4219 bit 4 → D-pad up         */
  SN_DOWN   = 0x004, /* bit 2  → $4219 bit 5 → D-pad down       */
  SN_LEFT   = 0x002, /* bit 1  → $4219 bit 6 → D-pad left        */
  SN_RIGHT  = 0x001, /* bit 0  → $4219 bit 7 → D-pad right      */
};

/* Start workaround: the game's BIT #$0010 test checks output bit 4,
 * which after SwapInputBits = input bit 11. When Start is pressed,
 * this bit must also be set so the game detects the Start button. */
#define SN_START_WORKAROUND 0x800
