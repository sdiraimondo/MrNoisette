cmake_minimum_required(VERSION 3.20)
project(MrNutzRecomp C)

set(CMAKE_C_STANDARD 11)

set(SNESRECOMP_ROOT "${CMAKE_SOURCE_DIR}/snesrecomp" CACHE PATH
    "Path to the snesrecomp checkout used by this game")

include(${SNESRECOMP_ROOT}/runner/runner.cmake)

file(GLOB GENERATED_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/generated/*.c)

# Game-specific source files, organized by module
set(GAME_SOURCES
    # Application
    src/app/main.c

    # Platform abstraction
    src/platform/window.c
    src/platform/audio_out.c
    src/platform/input.c
    src/platform/config.c

    # Mr Nutz game-specific
    src/mrnutz/game.c
    src/mrnutz/frame.c
    src/mrnutz/ppu_render.c
    src/mrnutz/mrnutz_spc_player.c

    # Audio modules (MP3 music replacement)
    src/audio/mp3_decoder.c
    src/audio/mp3_player.c
    src/audio/music_manager.c
    src/audio/audio_mixer.c

    # Level modules (custom levels)
    src/levels/level_reader.c
    src/levels/level_injector.c
    src/levels/level_browser.c

    # Overlays
    src/overlay/fps.c
    src/overlay/frame_dump.c

    # Generated recompiled code
    ${GENERATED_SOURCES}
)

add_executable(MrNutzRecomp
    ${SNESRECOMP_RUNNER_SOURCES}
    ${GAME_SOURCES}
)

target_include_directories(MrNutzRecomp PRIVATE
    ${CMAKE_SOURCE_DIR}/recomp
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/app
    ${CMAKE_SOURCE_DIR}/src/platform
    ${CMAKE_SOURCE_DIR}/src/mrnutz
    ${CMAKE_SOURCE_DIR}/src/audio
    ${CMAKE_SOURCE_DIR}/src/levels
    ${CMAKE_SOURCE_DIR}/src/overlay
    ${SNESRECOMP_RUNNER_INCLUDE_DIRS}
)

snesrecomp_target_fiber_compat(MrNutzRecomp)
snesrecomp_target_post_mortem(MrNutzRecomp TIER2)
snesrecomp_target_mmx_config(MrNutzRecomp)
snesrecomp_target_sdl(MrNutzRecomp)

# MP3 decoder (minimp3 single-header library)
target_include_directories(MrNutzRecomp PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

target_compile_definitions(MrNutzRecomp PRIVATE
    SNESRECOMP_TRACE=0
    SNESRECOMP_REVERSE_DEBUG=0
)

if(NOT MSVC)
    target_compile_options(MrNutzRecomp PRIVATE
        -w -Wno-implicit-function-declaration)
endif()

target_link_libraries(MrNutzRecomp PRIVATE
    ${SNESRECOMP_RUNNER_LIBRARIES}
)

if(NOT WIN32)
    target_link_libraries(MrNutzRecomp PRIVATE m)
endif()
