/* minimp3.h — stub for compilation without the real library.
 * Download from: https://github.com/lieff/minimp3/raw/master/minimp3.h
 * Replace this file with the real minimp3.h (public domain, single-header). */
#ifndef MINIMP3_H_STUB
#define MINIMP3_H_STUB

#include <stdint.h>
#include <string.h>

typedef struct { uint8_t dummy[512]; } mp3dec_t;
typedef struct { int frame_bytes; int hz; int channels; } mp3dec_frame_info_t;

static inline void mp3dec_init(mp3dec_t *d) { memset(d, 0, sizeof(*d)); }
static inline int mp3dec_decode_frame(mp3dec_t *d, const uint8_t *buf, size_t buf_size,
                                      int16_t *pcm, mp3dec_frame_info_t *info) {
    (void)d; (void)buf; (void)buf_size; (void)pcm;
    info->frame_bytes = 0; info->hz = 44100; info->channels = 2;
    return 0; /* no samples decoded */
}

#endif
