#pragma once
#include <stdint.h>
#include <stdbool.h>

/* Audio module: MP3 decoder (stub — implementation will use minimp3).
 * Decodes MP3 files to 44100 Hz stereo PCM samples. */

typedef struct Mp3Decoder Mp3Decoder;

Mp3Decoder *Mp3Decoder_Create(void);
void Mp3Decoder_Destroy(Mp3Decoder *dec);

/* Opens an MP3 file for decoding. Returns true on success. */
bool Mp3Decoder_OpenFile(Mp3Decoder *dec, const char *path);

/* Decodes the next chunk of samples into the output buffer.
 * Returns the number of samples decoded, or 0 at end of stream. */
int Mp3Decoder_Read(Mp3Decoder *dec, int16_t *out, int max_samples);

/* Seeks to the beginning of the file. */
void Mp3Decoder_Rewind(Mp3Decoder *dec);
