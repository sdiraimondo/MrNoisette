#pragma once
#include "mrnutz/frame.h"
#include "mrnutz/game.h"
#include "mrnutz/ppu_render.h"
#include "mrnutz/mrnutz_spc_player.h"
#include "platform/window.h"
#include "platform/audio_out.h"
#include "platform/input.h"
#include "platform/config.h"
#include "overlay/fps.h"
#include "overlay/frame_dump.h"
#include "audio/music_manager.h"

/* Entry point. Initializes all subsystems and runs the main loop. */
int main(int argc, char **argv);
