#include "audio/mp3_decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MP3 decoder implementation using minimp3 (single-header library).
 * minimp3.h must be placed in third_party/ and included here with
 * MINIMP3_IMPLEMENTATION defined to generate the implementation. */

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"

struct Mp3Decoder {
  mp3dec_t dec;
  FILE *file;
  uint8_t *file_data;
  size_t file_size;
  size_t read_pos;
  int sample_rate;
  int channels;
};

Mp3Decoder *Mp3Decoder_Create(void) {
  return calloc(1, sizeof(Mp3Decoder));
}

void Mp3Decoder_Destroy(Mp3Decoder *dec) {
  if (!dec) return;
  if (dec->file) fclose(dec->file);
  free(dec->file_data);
  free(dec);
}

bool Mp3Decoder_OpenFile(Mp3Decoder *dec, const char *path) {
  if (!dec) return false;
  /* Clean up previous file */
  if (dec->file) { fclose(dec->file); dec->file = NULL; }
  free(dec->file_data);
  dec->file_data = NULL;
  dec->file_size = 0;
  dec->read_pos = 0;

  /* Read entire file into memory (MP3 files are small enough) */
  FILE *f = fopen(path, "rb");
  if (!f) return false;
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (sz <= 0) { fclose(f); return false; }
  dec->file_data = malloc(sz);
  if (!dec->file_data) { fclose(f); return false; }
  if (fread(dec->file_data, 1, sz, f) != (size_t)sz) {
    free(dec->file_data); dec->file_data = NULL;
    fclose(f); return false;
  }
  fclose(f);
  dec->file_size = sz;
  dec->read_pos = 0;

  /* Initialize minimp3 */
  mp3dec_init(&dec->dec);
  dec->sample_rate = 0;
  dec->channels = 0;
  return true;
}

int Mp3Decoder_Read(Mp3Decoder *dec, int16_t *out, int max_samples) {
  if (!dec || !dec->file_data || dec->read_pos >= dec->file_size)
    return 0;

  mp3dec_frame_info_t info;
  int total = 0;
  while (total < max_samples) {
    int samples = mp3dec_decode_frame(&dec->dec,
                                       dec->file_data + dec->read_pos,
                                       dec->file_size - dec->read_pos,
                                       out + total,
                                       &info);
    dec->read_pos += info.frame_bytes;
    if (samples == 0) {
      if (info.frame_bytes == 0) break; /* end of stream */
      continue; /* skipped frame */
    }
    if (dec->sample_rate == 0) {
      dec->sample_rate = info.hz;
      dec->channels = info.channels;
    }
    total += samples * info.channels;
    if (dec->read_pos >= dec->file_size) break;
  }
  return total;
}

void Mp3Decoder_Rewind(Mp3Decoder *dec) {
  if (!dec) return;
  dec->read_pos = 0;
  if (dec->file_data)
    mp3dec_init(&dec->dec);
}
