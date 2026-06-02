#include "headers/helperGUI.h"
#include <stdio.h>

int guiInit(GUI *gui, const char *title, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return 0;
    }

    gui->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        0
    );

    if (!gui->window) {
        printf("Window Error: %s\n", SDL_GetError());
        return 0;
    }

    gui->renderer = SDL_CreateRenderer(gui->window, -1, SDL_RENDERER_ACCELERATED);

    if (!gui->renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        return 0;
    }

    gui->running = 1;

    return 1;
}

void guiClear(GUI *gui, int r, int g, int b, int a)
{
    SDL_SetRenderDrawColor(gui->renderer, r, g, b, a);
    SDL_RenderClear(gui->renderer);
}

void guiDrawRect(GUI *gui, int x, int y, int w, int h, int r, int g, int b, int a)
{
    SDL_Rect rect = {x, y, w, h};

    SDL_SetRenderDrawColor(gui->renderer, r, g, b, a);
    SDL_RenderFillRect(gui->renderer, &rect);
}

void guiPresent(GUI *gui)
{
    SDL_RenderPresent(gui->renderer);
}

void guiHandleEvents(GUI *gui)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            gui->running = 0;
        }
    }
}

void guiDestroy(GUI *gui)
{
    SDL_DestroyRenderer(gui->renderer);
    SDL_DestroyWindow(gui->window);
    SDL_Quit();
}