#include <algorithm>
#include <iostream>
#include <SDL.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int main(int argc, char* argv[])
{
    // Initialize SDL, window, screen, renderer, and texture
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Fractal", 20, 20, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Surface *surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING | SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Get pointers to start and end of pixels buffer
    uint32_t *surfacePixelsStart = reinterpret_cast<uint32_t *>(surface->pixels);
    uint32_t *surfacePixelsEnd = surfacePixelsStart + SCREEN_WIDTH * SCREEN_HEIGHT - 1;

    // Modify buffer
    SDL_LockSurface(surface);
    std::fill(surfacePixelsStart, surfacePixelsEnd, 0x00500000); // Copy dark green to all pixels in buffer
    SDL_UnlockSurface(surface);

    // Update Surface
    SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // Wait for quit
    SDL_Event event;
    SDL_PollEvent(&event);
    while(event.type != SDL_QUIT)
        SDL_PollEvent(&event);
}
