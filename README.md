# Mr. Nutz (SNES) Recompilation Project (WIP)

A native recompilation of the SNES game *Mr. Nutz* using the [snesrecomp](https://github.com/mstan/snesrecomp) framework.

---

## Global Status
The game launches and is playable! (with some occasional slowdowns). A lot of work is still necessary to fix the various bugs and slowdowns (see below)

--

## What Remains To Do

| Item | Status | Priority |
|------|--------|----------|
| Gamepad mapping correction | Pending — current defaults don't match Mr Nutz controls | Medium |
| Title screen cursor blink | Rolled back — manual pixel clearing caused full-screen flicker. Needs a targeted approach (only clear the selected menu item's text, not entire rows) | Low |
| Framerate stutter investigation | Occasional stutter despite stable FPS counter. Likely interpreter tier stepping cap resets on memory writes | Low |
| Enrich bank configs | 219 dispatch miss sites in the tier2 log. Promoting hot functions to AOT would improve performance | Future |
| Co-simulation | Enable SNES_COSIM to validate against snes9x | Future |

---

## Project Structure

```
MrNoisette/
  ROM/                     Original game ROM (Mr. Nutz (USA) (En,Fr).sfc)
  Output/                  Recompilation project root
    snesrecomp/             Framework submodule (git submodule)
    recomp/                 Bank configuration & function declarations
      bank00.cfg            Recompiler config for bank $00 (vectors, funcs)
      funcs.h               C function declarations for generated code
    src/                    Game-specific host & integration code
      main.c                SDL3 host: window, audio, input, main loop
      mrnutz_rtl.c          Frame execution: reset, NMI, main loop, PPU
      mrnutz_rtl.h          Public declarations for RTL functions
      mrnutz_cpu_infra.c    RtlGameInfo registration struct
      mrnutz_spc_player.c   SPC audio player (DSP init, SPC upload)
      mrnutz_spc_player.h   SPC player header
      variables.h           Game-specific WRAM variable aliases
      banks.h               v2 ABI documentation (no macros needed)
    generated/              Auto-generated recompiled C (gitignored)
      bank00_v2.c … bank9d_v2.c   Per-bank recompiled functions
      dispatch_v2.c               Dispatch table for indirect calls
      unresolved_stubs_v2.c       Stubs for unresolved references
    CMakeLists.txt          Build configuration
    config.ini             Runtime config: graphics, sound, [Keyboard], [Gamepad]
    build.sh               Linux/macOS build script
    build.ps1              Windows build script
    .gitmodules            Submodule declaration (snesrecomp)
```

---

## What Has Been Accomplished

### 1. Project Setup
- Cloned snesrecomp as a git submodule at `snesrecomp/`
- Built the native Rust analyzer (`recompiler-rs/target/release/snesrecomp-analyze`)
- Generated initial recompiled C output from the ROM via `snesrecomp_cli.py`
- Created project structure modeled on MegamanXSNESRecomp

### 2. Game Execution
- **Reset handler**: Runs from SNES reset vector ($00:80B5) via `interp_bridge_run_until_quiescent` until the main loop's WAI yields
- **Per-frame NMI**: Advances PPU beam to vblank (scanline 225), pushes interrupt frame, runs NMI handler ($00:8349) via `interp_bridge_run_interrupt`, then resumes main loop via `interp_bridge_run_until_quiescent` until next WAI
- **PPU beam fix**: The NMI handler reads H/V counters ($2137/$213D) to determine its code path. Without advancing the beam to vblank, the counters return 0, causing a ~240x slowdown in the fade-in
- **HDMA processing**: Added per-scanline HDMA in `MrNutzDrawPpuFrame` (channels 5–7) for per-line effects

### 3. Display
- **SDL_PIXELFORMAT_XRGB8888**: The PPU writes BGRA pixels with alpha=0. Using XRGB8888 + SDL_BLENDMODE_NONE makes SDL ignore alpha, fixing the black screen issue
- **Vsync off + manual 60fps pacer**: Vsync was rounding frame time to multiples of 16.67ms, causing 30fps. Disabling vsync and manually pacing to 16ms achieves 60fps
- **FPS counter**: Toggle with F key or via window title

### 4. Input System
- **config.ini sections `[Keyboard]` and `[Gamepad]`**: Both active simultaneously (OR-ed). One line per SNES button, editable via INI
- **SwapInputBits workaround**: The framework's `SwapInputBits()` reverses all 16 bits of the controller value. Each SNES button maps to a unique
  bit in the 12-bit input word. Start needs an extra bit (0x800) because the game's `BIT #$0010` test checks output bit 4 = input bit 11
- **Gamepad support**: SDL3 gamepad API, auto-opened on startup
  
### 5. Audio
- SPC player with 64KB RAM, DSP init, and length-prefixed SPC upload
- SDL3 audio stream at 44100 Hz, 2 channels
- Audio thread runs independently via SDL callback

---

## Source File Documentation

### `src/main.c` — SDL3 Host Application (609 lines)

This is the main entry point. It initializes SDL, loads the ROM,
creates the audio system, opens gamepads, and runs the main loop.

#### Functions

```c
void NORETURN Die(const char *error)
```
Fatal error handler. Reports to host_report_fatal, prints to stderr,
and exits. Required by the framework (util.c calls Die on allocation
failure).

```c
void MkDir(const char *s)
```
Cross-platform directory creation. Required by the framework
(RtlReadSram/RtlWriteSram call it for the saves/ directory).

```c
static void SaveFrameBMP(const char *dir, int frame_num, uint8 *pixels, int w, int h)
```
Saves a single frame as a 32-bit BMP file. Used by the --framedump
command-line option for capturing intro demo screenshots.

```c
void RtlApuLock(void) / void RtlApuUnlock(void)
```
Audio thread synchronization. Wraps SDL_LockMutex/UnlockMutex.
Required by the framework's audio rendering (RtlRenderAudio).

```c
static void FillAudioBuffer(Uint8 *stream, int len)
```
Fills the SDL audio stream buffer from the SPC audio ring buffer.
When the ring buffer is exhausted, calls RtlRenderAudio to produce
one block of SPC audio samples.

```c
static void SDLCALL AudioStreamCallback(...)  // SDL3
static void SDLCALL AudioCallback(...)         // SDL2
```
SDL audio callback wrappers. SDL3 uses a stream-based API; SDL2 uses
a pull callback. Both delegate to FillAudioBuffer.

```c
static int FindNameVal(const NameVal *table, const char *name)
```
Looks up a name in a NameVal table (case-insensitive). Returns the
associated integer value or -1 if not found.

```c
static void ParseBindList(const char *str, const NameVal *table, BindList *out)
```
Parses a comma-separated list of key/button names into a BindList.
Each name is looked up in the given NameVal table.

```c
static void LoadConfig(const char *filename)
```
Loads config.ini. Parses [Keyboard] and [Gamepad] sections to build
the keybind tables. Also initializes default bindings if the file
is missing or a section is absent.

```c
static void HandleInput(int key, bool pressed)
```
Handles hotkey events (P=pause, Tab=turbo, F=FPS toggle, F1/F2 =
save/load state). Does NOT handle game buttons — those are polled
via PollInputState.

```c
static void PollInputState(void)
```
Polls keyboard (SDL_GetKeyboardState) and gamepad
(SDL_GetGamepadButton) every frame. OR-ed together. Applies the
Start workaround (sets bit 0x800 alongside SN_START). Logs changes
to stderr.

```c
static void RenderDigit(uint8 *dst, size_t pitch, int digit, int x0, int y0, uint32 color)
```
Draws a single digit (0-9) at (x0,y0) using a 5x7 pixel font directly
into the framebuffer. Used by the FPS overlay.

```c
static void RenderNumber(uint8 *dst, size_t pitch, int n, int x0, int y0, uint32 color)
```
Draws a multi-digit integer at (x0,y0) by calling RenderDigit for
each digit.

```c
static void DrawPpuFrame(void)
```
Renders one PPU frame to g_my_pixels (256x224 ARGB). Calls
PpuBeginDrawing + ppu_runLine for each scanline, then optionally
overlays the FPS counter, uploads to the SDL texture, and presents.

```c
int main(int argc, char **argv)
```
Entry point. Parses --framedump arguments, resolves the ROM path,
loads config.ini, registers the game with the framework, initializes
SDL (video+audio+gamepad), creates the window/renderer/texture,
opens the audio device, opens the first gamepad, and enters the
main loop. The main loop: polls events, polls input state, calls
RtlRunFrame, renders the PPU frame, updates the FPS counter, and
paces to 60fps.

---

### `src/mrnutz_rtl.c` — Frame Execution & PPU Rendering (69 lines)

This file implements the per-frame game logic driver and the PPU
frame renderer with HDMA support.

```c
void RunOneFrameOfGame(void)
```
Called by the framework's RtlRunFrame every frame. On the first call,
initializes the CPU state and runs the reset handler from $00:80B5
until WAI. On subsequent calls: advances the PPU beam to vblank
(scanline 225), pushes an NMI interrupt frame, runs the NMI handler
via interp_bridge_run_interrupt, then resumes the main loop via
interp_bridge_run_until_quiescent until the next WAI yield. Tracks
the resume PC between frames.

```c
void MrNutzDrawPpuFrame(void)
```
Renders the PPU to g_my_pixels with per-scanline HDMA processing.
Initializes SimpleHdma channels 5-7 from the DMA controller, then
for each scanline: calls ppu_runLine followed by SimpleHdma_DoLine
for each channel. This is required for per-line effects (gradients,
windowing, color math) that the game sets up via HDMA tables.

---

### `src/mrnutz_cpu_infra.c` — Game Registration (15 lines)

```c
const RtlGameInfo kMrNutzGameInfo
```
The RtlGameInfo struct that registers Mr Nutz with the framework.
Fields:
- .title = "mrnutz" — used for save paths and diagnostics
- .initialize = NULL — no special init hook
- .run_frame = &RunOneFrameOfGame — per-frame game logic
- .draw_ppu_frame = &MrNutzDrawPpuFrame — PPU rendering
- .save_name_prefix = "save" — savestate filename prefix
- All savestate hooks (state_save_extra, etc.) = NULL

---

### `src/mrnutz_spc_player.c` — SPC Audio Player (66 lines)

```c
static void Dsp_Write(MrNutzSpcPlayer *p, uint8_t reg, uint8 value)
```
Writes a value to a DSP register via the framework's dsp_write.

```c
static void Spc_Reset(MrNutzSpcPlayer *p)
```
Resets the SPC player: zeroes SPC RAM (first 0x500 bytes), clears
input ports, and writes default DSP register values (volume, flags,
echo, etc.).

```c
static void MrNutzSpcPlayer_Initialize(SpcPlayer *p_in)
```
Called once at startup. Resets the DSP and SPC player state.

```c
static void MrNutzSpcPlayer_Upload(SpcPlayer *p_in, const uint8_t *data)
```
Uploads SPC data from the ROM into SPC RAM. Parses the length-prefixed
chunk format: {uint16 numbytes, uint16 target_addr, data...} until
numbytes=0. Mutes the DSP during upload, then unmutes.

```c
SpcPlayer *MrNutzSpcPlayer_Create(void)
```
Allocates and initializes a MrNutzSpcPlayer (64KB SPC RAM + DSP).
Installs the initialize and upload callbacks. Returns the SpcPlayer
base pointer.

---

### `src/mrnutz_rtl.h` — Public Declarations (8 lines)

Declares `kMrNutzGameInfo`, `RunOneFrameOfGame`, and
`MrNutzDrawPpuFrame` for use by main.c and mrnutz_cpu_infra.c.

---

### `src/mrnutz_spc_player.h` — SPC Player Header (4 lines)

Declares `MrNutzSpcPlayer_Create()`.

---

### `src/variables.h` — WRAM Variable Aliases (11 lines)

```c
extern uint16 counter_global_frames
```
Host-side frame counter, incremented once per RtlRunFrame.

```c
#define waiting_for_vblank (*(uint8*)(g_ram + 0x0B9D))
```
The game's NMI vblank flag at WRAM $0B9D. The game's main loop
spin-waits on this location.

---

### `src/banks.h` — v2 ABI Documentation (8 lines)

Documents that the v2 ABI uses plain CpuState* parameters with no
aggregate typedefs or RECOMP_BANK macros.

---

### `recomp/bank00.cfg` — Recompiler Configuration (33 lines)

Configures the recompiler for bank $00:
- `auto_vectors` — reads interrupt vectors from ROM $00:FFE0-FFFF
- `tier_down_stubs` — unresolved cross-bank calls run on the
  interpreter tier instead of being dropped
- `func ResetHandler 80b5` — declares the reset handler entry
- `func NmiHandler 8349` — declares the NMI handler entry
- `func IrqHandler 83b3` — declares the IRQ handler entry
- `name 8080b5 ResetHandler` etc. — aliases for $80-bank LoROM mirrors

---

### `recomp/funcs.h` — Generated Code Declarations (17 lines)

Declares `I_RESET`, `I_NMI`, `I_IRQ` (auto-discovered interrupt
handlers) and `WatchdogCheck` (framework hook). Included by all
generated bank C files.

---

### `CMakeLists.txt` — Build Configuration (49 lines)

- Includes `snesrecomp/runner/runner.cmake` for framework sources
- Globs `generated/*.c` for recompiled bank sources
- Compiles `src/main.c`, `src/mrnutz_rtl.c`, `src/mrnutz_cpu_infra.c`,
  `src/mrnutz_spc_player.c` as game-specific sources
- Links against SDL3 (via snesrecomp_target_sdl)
- Enables fiber compat, post-mortem (Tier2), and MMX config helpers
- Suppresses warnings on generated code (-w -Wno-implicit-function-declaration)

---

### `config.ini` — Runtime Configuration (50 lines)

Sections:
- `[General]` — DisableFrameDelay, SkipLauncher
- `[Graphics]` — Fullscreen, WindowScale, NewRenderer, DisplayAspect
- `[Sound]` — EnableAudio, AudioFreq, AudioChannels, AudioSamples
- `[Keyboard]` — One line per SNES button: `ButtonName = KeyName`
- `[Gamepad]` — One line per SNES button: `ButtonName = GamepadBtnName`

---

## Build Instructions

Prerequisites: CMake, Ninja, a C compiler, SDL3.

```sh
sh build.sh
```

The result is `build/MrNutzRecomp`.

## Run

```sh
./build/MrNutzRecomp "path/to/Mr. Nutz (USA) (En,Fr).sfc"
```

## Controls

Hotkeys: F=FPS toggle, P=Pause, Tab=Turbo, F1=Save state, F2=Load state.
Game buttons are configured in `config.ini` under `[Keyboard]` and `[Gamepad]`.

## Architecture Notes

- The game runs primarily on the interpreter tier (only interrupt
  vectors are statically recompiled). Performance is ~60fps on a
  modern CPU due to the manual frame pacer.
- `SwapInputBits` in the framework reverses all 16 bits of the
  controller value. Each SNES button maps to a unique bit in the
  12-bit input word, with Start requiring an additional bit (0x800)
  for the game's `BIT #$0010` test.
- `generated/` is derived from copyrighted ROM data. Do not
  redistribute unless you have permission.
