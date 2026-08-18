#include "overlay/frame_dump.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

static const char *s_dir = NULL;
static int s_count = 0;
static int s_max = 0;
static int s_skip = 0;

void BmpDump_Init(const char *dir, int skip, int max) {
  s_dir = dir; s_count = 0; s_skip = skip; s_max = max;
}

bool BmpDump_IsActive(void) {
  return s_dir && s_count < s_max;
}

void BmpDump_Capture(uint8 *pixels, int w, int h, int frame_num) {
  if (!s_dir || s_count >= s_max || frame_num < s_skip) return;
  char path[512];
  snprintf(path, sizeof(path), "%s/frame_%06d.bmp", s_dir, s_count);
  FILE *f = fopen(path, "wb");
  if (!f) return;
  int row_bytes = w * 4;
  int file_size = 14 + 40 + row_bytes * h;
  uint8 header[54] = {0};
  header[0]='B'; header[1]='M';
  header[2]=file_size&0xFF; header[3]=(file_size>>8)&0xFF;
  header[4]=(file_size>>16)&0xFF; header[5]=(file_size>>24)&0xFF;
  header[10]=54; header[14]=40;
  header[18]=w&0xFF; header[19]=(w>>8)&0xFF;
  header[22]=h&0xFF; header[23]=(h>>8)&0xFF;
  header[26]=1; header[28]=32;
  fwrite(header, 1, 54, f);
  for (int y = h-1; y >= 0; y--)
    fwrite(pixels + y*row_bytes, 1, row_bytes, f);
  fclose(f);
  s_count++;
}
