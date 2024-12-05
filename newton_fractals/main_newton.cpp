#include <SDL2/SDL.h>
#include <complex>
#include <iostream>
#include <chrono>
#include "newton_fractal.h"
#include <cstring>
#include <omp.h>
#include "reframe.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int main(){

    // Initialize SDL, window, screen, renderer, and texture
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Newton's Fractal", 20, 20, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Surface *screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING | SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

    int xMouse, yMouse;         // Mouse position
    int update = 1;             // Update frame
    float zoomInRatio = 0.5;    // Amount to zoom in by
    float zoomOutRatio = -1.0;  // Amount to zoom out by
    uint32_t buttons;             // buttons to parse
    const uint8_t* keys = SDL_GetKeyboardState(NULL);    // keys to parse
    int omp_imp = 1;            // use cuda implementation
    int j;                      // iterator in CPU implementation

    // Default bounds
    float* boundsHost = (float*)malloc(4*sizeof(float));
    float xLowerBound = -2.21f;
    float xUpperBound = 1.63f;
    float yLowerBound = -1.2f;
    float yUpperBound = 1.2f;
    float xScale = (xUpperBound - xLowerBound) / SCREEN_WIDTH;
    float yScale = (yUpperBound - yLowerBound) / SCREEN_HEIGHT;

    // Fill bounds and send to device
    boundsHost[0] = xLowerBound;
    boundsHost[1] = yLowerBound;
    boundsHost[2] = xScale;
    boundsHost[3] = yScale;

    printf("\nLaunching CPU implementation.\n");
    printf("Mouse interaction: Left click to zoom in, Right click to zoom out.\n");

    SDL_Event event;
    bool eventOccurred = SDL_PollEvent(&event);

    while (event.type != SDL_QUIT) {
        if(eventOccurred) {
            switch (event.type) {
                case SDL_MOUSEWHEEL: { // Use a block scope here
                    int buttons = SDL_GetMouseState(&xMouse, &yMouse);
                    if (event.wheel.y > 0) {
                        // Scroll up: zoom in
                        update = 1;
                        reframe(zoomInRatio, xMouse, yMouse, SCREEN_WIDTH, SCREEN_HEIGHT,
                                &xScale, &yScale, &xLowerBound, &xUpperBound,
                                &yLowerBound, &yUpperBound);
                    } else if (event.wheel.y < 0) {
                        // Right click: zoom out
                        update = 1;
                        reframe(zoomOutRatio, xMouse, yMouse, SCREEN_WIDTH, SCREEN_HEIGHT,
                                &xScale, &yScale, &xLowerBound, &xUpperBound,
                                &yLowerBound, &yUpperBound);
                    }
                    break;
                }

                case SDL_KEYDOWN: {
                    const uint8_t* keys = SDL_GetKeyboardState(NULL);
                    if (keys[SDL_SCANCODE_S]) {
                        // Switch to another implementation if needed
                        update = 1;
                        if(omp_imp){
                            printf("\nSwitching to Single-Threaded Implementation...\n");
                            omp_imp = 0;
                        }
                        else{
                            printf("\nSwitching to Multi-Threaded Implementation...\n");
                            omp_imp = 1;
                        }
                        std::cout << "S key pressed" << std::endl;
                    }
                    break;
                }

                default:
                    break;
            }
        }

        // If there was an update, recompute the fractal
        if (update) {

            printf("Recomputing fractal with new bounds...\n");
            update = 0; // Reset update flag

            // Create a buffer for storing pixel colors
            std::vector<uint32_t> pixelBuffer(SCREEN_WIDTH * SCREEN_HEIGHT); // Buffer for a single line

            // Time Frame Rendering
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

            #pragma omp parallel for if(omp_imp)
            for (int i = 0; i < SCREEN_HEIGHT; i++) {
                for (int k = 0; k < SCREEN_WIDTH; k++) {

                    // Map pixel to complex plane
                    std::complex<float> z(xLowerBound + k * xScale, yLowerBound + i * yScale);

                    // Compute Newton
                    const int j = newton(z);

                    // Assign color based on the root and iteration count
                    uint32_t brightness = 255.0f * std::max(0.1f, 1.0f - (float)j / MAX_ITERATIONS);
                    if (j < MAX_ITERATIONS) {

                        if (std::abs(z - std::complex<float>(1, 0)) < EPSILON)
                            brightness <<= 24; // Red for Root 1
                        
                        else if (std::abs(z - std::complex<float>(-0.5, std::sqrt(3) / 2)) < EPSILON)
                            brightness <<= 16; // Green for Root 2
                        
                        else if (std::abs(z - std::complex<float>(-0.5, -std::sqrt(3) / 2)) < EPSILON)
                            brightness <<= 8; // Blue for Root 3
                    }

                    // Store color in the line buffer
                    pixelBuffer[i * SCREEN_WIDTH + k] = brightness | SDL_ALPHA_OPAQUE;
                }
            }

            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            // Modify buffer
            SDL_LockSurface(screen);
            std::memcpy(screen->pixels, &pixelBuffer.at(0), sizeof(uint32_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
            SDL_UnlockSurface(screen);

            // Update Surface
            SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            std::cout << "Frame Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " us\n";
        }

        eventOccurred = SDL_PollEvent(&event);
    }

    // Quit
    std::cout << "exiting...\n";
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
