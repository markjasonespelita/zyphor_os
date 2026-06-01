#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int running;
} GUI;

int guiInit(GUI *gui, const char *title, int width, int height);
void guiClear(GUI *gui, int r, int g, int b, int a);
void guiDrawRect(GUI *gui, int x, int y, int w, int h, int r, int g, int b, int a);
void guiPresent(GUI *gui);
void guiHandleEvents(GUI *gui);
void guiDestroy(GUI *gui);

#endif