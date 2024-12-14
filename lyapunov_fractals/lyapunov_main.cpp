#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <omp.h>
#include "lyapunov_fractal.h"
#include "reframe.h"
#include "render_cuda.h"

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 900

int main(int argc, char* argv[]) {
    // Check if a sequence is provided as input
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <sequence>\n";
        std::cerr << "Example: " << argv[0] << " AABAB\n";
        return 1;
    }

    int imp = 0;
    if (argc > 2) imp = 1; // Use CUDA if second arg given

    std::string sequence = argv[1];
    if (sequence.empty()) {
        std::cerr << "Error: The input sequence cannot be empty.\n";
        return 1;
    }

    // Validate input sequence (must contain only 'A' and 'B')
    for (char c : sequence) {
        if (c != 'A' && c != 'B') {
            std::cerr << "Error: The sequence must contain only 'A' and 'B'.\n";
            return 1;
        }
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Lyapunov Fractal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );
    SDL_Surface* screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Image plane bounds
    float aMin = 2.0f, aMax = 4.0f;
    float bMin = 2.0f, bMax = 4.0f;

    // Scale factors
    float aScale = (aMax - aMin) / SCREEN_WIDTH;
    float bScale = (bMax - bMin) / SCREEN_HEIGHT;

    int xMouse, yMouse;
    float zoomInRatio = 0.2f;
    float zoomOutRatio = -0.2f;
    int update = 1; // Flag to recompute fractal

    SDL_Event event;
    bool running = true;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_MOUSEWHEEL: {
                int buttons = SDL_GetMouseState(&xMouse, &yMouse);
                if (event.wheel.y > 0) {
                    // Zoom in
                    update = 1;
                    reframeLyapunov(zoomInRatio, xMouse, yMouse, SCREEN_WIDTH, SCREEN_HEIGHT,
                            &aScale, &bScale, &aMin, &aMax, &bMin, &bMax);
                } else if (event.wheel.y < 0) {
                    // Zoom out
                    update = 1;
                    reframeLyapunov(zoomOutRatio, xMouse, yMouse, SCREEN_WIDTH, SCREEN_HEIGHT,
                            &aScale, &bScale, &aMin, &aMax, &bMin, &bMax);
                }
                break;
            }


                default:
                    break;
            }
        }

        // Recompute the fractal if needed
        if (update) {
            update = 0; // Reset update flag
            std::vector<uint32_t> pixelBuffer(SCREEN_WIDTH * SCREEN_HEIGHT);

            auto startTime = std::chrono::high_resolution_clock::now();

            // Compute Lyapunov fractal
            #pragma omp parallel for schedule(dynamic)
            for (int y = 0; y < SCREEN_HEIGHT; ++y) {
                for (int x = 0; x < SCREEN_WIDTH; ++x) {
                    float a = aMin + x * aScale;
                    float b = bMin + y * bScale;

                    float lyapunov = computeLyapunov(sequence, a, b);
                    pixelBuffer[y * SCREEN_WIDTH + x] = mapLyapunovToColor(lyapunov);
                }
            }

    // Stop timer
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Fractal computed in " << duration << " ms\n";

    } else
        renderCuda(pixelBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, aMin, bMin, aScale, bScale, sequence);

    // Render the fractal
    SDL_UpdateTexture(texture, nullptr, pixelBuffer.data(), SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    // Wait for user to close the window
    SDL_Event event;
    SDL_PollEvent(&event);
    while(event.type != SDL_QUIT)
        SDL_PollEvent(&event);

    // Clean up SDL
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
