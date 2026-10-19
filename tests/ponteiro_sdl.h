// SDL de entrada dublado: sem SDL_Init/Cocoa/dispositivo real. So ponteiro.c.
#ifndef NV_TEST_PONTEIRO_SDL_H
#define NV_TEST_PONTEIRO_SDL_H
#include <SDL2/SDL.h>
#include <string.h>
void *SDL_memset(void *s, int c, size_t n) { return memset(s, c, n); }
Uint32 SDL_GetTicks(void) { return 0; }
int SDL_GetNumTouchDevices(void) { return 0; }
SDL_Window *SDL_GL_GetCurrentWindow(void) { return NULL; }
SDL_Window *SDL_GetWindowFromID(Uint32 id) { (void)id; return NULL; }
SDL_Window *SDL_GetMouseFocus(void) { return NULL; }
void SDL_GetWindowSize(SDL_Window *w, int *larg, int *alt) {
  (void)w; if (larg) *larg = 1920; if (alt) *alt = 1080;
}
void SDL_GL_GetDrawableSize(SDL_Window *w, int *larg, int *alt) { SDL_GetWindowSize(w, larg, alt); }
Uint32 SDL_GetMouseState(int *x, int *y) { if (x) *x = 0; if (y) *y = 0; return 0; }
int SDL_ShowCursor(int toggle) { (void)toggle; return SDL_DISABLE; }
#endif
