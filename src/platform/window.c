#include "platform/window.h"
#include "desktop/sdl_compat.h"

static const char kWindowTitle[] = "Mr. Nutz (Recompiled)";
static SDL_Window *g_window;
static SDL_Renderer *g_renderer;
static SDL_Texture *g_texture;
int g_snes_width = 256;
int g_snes_height = 224;
uint8 g_my_pixels[256 * 4 * 240];

bool Window_Init(int scale) {
  g_window = SDL_CreateWindow(kWindowTitle,
                               256 * scale, g_snes_height * scale,
                               SDL_WINDOW_RESIZABLE);
  if (!g_window) return false;
  g_renderer = SDL_CreateRenderer(g_window, NULL);
  if (!g_renderer) return false;
#if SNESRECOMP_SDL3
  SDL_SetRenderVSync(g_renderer, 0);
#endif
  g_texture = SDL_CreateTexture(g_renderer,
#if SNESRECOMP_SDL3
                                SDL_PIXELFORMAT_XRGB8888,
#else
                                SDL_PIXELFORMAT_ARGB8888,
#endif
                                SDL_TEXTUREACCESS_STREAMING,
                                256, 240);
  if (!g_texture) return false;
#if SNESRECOMP_SDL3
  SDL_SetTextureBlendMode(g_texture, SDL_BLENDMODE_NONE);
#endif
  return true;
}

void Window_Present(void) {
  SDL_UpdateTexture(g_texture, NULL, g_my_pixels, g_snes_width * 4);
  SDL_RenderClear(g_renderer);
  int win_w, win_h;
  SDL_GetWindowSize(g_window, &win_w, &win_h);
  int sx = win_w / 256;
  int sy = win_h / g_snes_height;
  int s = sx < sy ? sx : sy;
  if (s < 1) s = 1;
  float dw = 256.0f * s, dh = (float)g_snes_height * s;
  SDL_FRect dst = { (float)(win_w-(int)dw)/2, (float)(win_h-(int)dh)/2, dw, dh };
#if SNESRECOMP_SDL3
  SDL_RenderTexture(g_renderer, g_texture, NULL, &dst);
#else
  SDL_RenderCopy(g_renderer, g_texture, NULL,
                 &(SDL_Rect){ (int)dst.x, (int)dst.y, (int)dst.w, (int)dst.h });
#endif
  SDL_RenderPresent(g_renderer);
}

void Window_SetTitle(const char *title) {
  if (g_window) SDL_SetWindowTitle(g_window, title);
}

void Window_Destroy(void) {
  if (g_texture) SDL_DestroyTexture(g_texture);
  if (g_renderer) SDL_DestroyRenderer(g_renderer);
  if (g_window) SDL_DestroyWindow(g_window);
}
