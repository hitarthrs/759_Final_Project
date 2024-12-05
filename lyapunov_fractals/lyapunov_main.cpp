#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <omp.h>
#include "lyapunov_fractal.h"

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 900

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Lyapunov Fractal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Sequence for the Lyapunov fractal
    std::string sequence = "AABAB";

    // Image plane bounds
    float aMin = 2.0f, aMax = 4.0f;
    float bMin = 2.0f, bMax = 4.0f;

    // Scale factors
    float aScale = (aMax - aMin) / SCREEN_WIDTH;
    float bScale = (bMax - bMin) / SCREEN_HEIGHT;

    // Frame buffer
    std::vector<uint32_t> pixelBuffer(SCREEN_WIDTH * SCREEN_HEIGHT);

    // Start timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // Parallel computation of Lyapunov fractal using OpenMP
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            float a = aMin + x * aScale;
            float b = bMin + y * bScale;

            // Compute Lyapunov exponent
            float lyapunov = computeLyapunov(sequence, a, b);

            // Map Lyapunov exponent to color
            pixelBuffer[y * SCREEN_WIDTH + x] = mapLyapunovToColor(lyapunov);
        }
    }

    // Stop timer
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Fractal computed in " << duration << " ms\n";

    // Render the fractal
    SDL_UpdateTexture(texture, nullptr, pixelBuffer.data(), SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    // Wait for user to close the window
    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    // Clean up SDL
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
