#include <SDL2/SDL.h>
#include <complex>
#include <iostream>
#include <chrono>
#include "newton_fractal.h"
#include <cstring>
#include <omp.h>
#include "reframe.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1200
int maxIter = 100;

// Polynomial and derivative for Newton's method (example: z^3 - 1)
std::complex<float> polynomial(std::complex<float> z) {
    return z * z * z - std::complex<float>(1.0f, 0.0f);
}

std::complex<float> derivative(std::complex<float> z) {
    return 3.0f * z * z;
}

float epsilon = 1e-5;

int main(){

    // Initialize SDL, window, screen, renderer, and texture

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Newton's Fractal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if(!window){
        printf("Error: Failed to open window\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Surface *screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if(!screen){
        printf("Error: Failed to create screen.\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer){
        printf("Error: Failed to create renderer\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING | SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if(!texture){
        printf("Error: Failed to create texture.\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    int xMouse, yMouse;         // Mouse position
    int update = 1;             // Update frame
    float zoomInRatio = 0.5;    // Amount to zoom in by
    float zoomOutRatio = -1.0;  // Amount to zoom out by
    Uint32 buttons;             // buttons to parse
    const Uint8* keys = SDL_GetKeyboardState(NULL);    // keys to parse
    int omp_imp = 0;            // use cuda implementation
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

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: // Exit loop
                running = false;
                break;

            case SDL_MOUSEBUTTONDOWN: { // Use a block scope here
                int buttons = SDL_GetMouseState(&xMouse, &yMouse);
                if ((buttons & SDL_BUTTON_LMASK) != 0) {
                    // Left click: zoom in
                    update = 1;
                    reframe(zoomInRatio, xMouse, yMouse, SCREEN_WIDTH, SCREEN_HEIGHT,
                            &xScale, &yScale, &xLowerBound, &xUpperBound,
                            &yLowerBound, &yUpperBound);
                } else if ((buttons & SDL_BUTTON_RMASK) != 0) {
                    // Right click: zoom out
                    update = 1;
                    reframe(zoomOutRatio, xMouse, yMouse, SCREEN_WIDTH, SCREEN_HEIGHT,
                            &xScale, &yScale, &xLowerBound, &xUpperBound,
                            &yLowerBound, &yUpperBound);
                }
                break;
            }

            case SDL_KEYDOWN: {
                const Uint8* keys = SDL_GetKeyboardState(NULL);
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
            // Time Frame-time
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

            if(omp_imp){

            float real, imag;
            Uint8 r, g, b;
            int j;
            std::complex<float> finalRoot;
            // Create a buffer for storing pixel colors            
            std::vector<Uint32> pixelBuffer(SCREEN_WIDTH * SCREEN_HEIGHT); // Buffer for a single line

            #pragma omp parallel for schedule(dynamic) private(real, imag, r, g, b, j, finalRoot)
            for (int i = 0; i < SCREEN_HEIGHT; i++) {
                for (int k = 0; k < SCREEN_WIDTH; k++) {
                    // Map pixel to complex plane
                    real = xLowerBound + k * xScale;
                    imag = yLowerBound + i * yScale;

                    // Compute iteration count
                    int j = newton(real, imag, maxIter, epsilon, polynomial, derivative);

                    // Start with initial point and iterate to find the converged root
                    std::complex<float> finalRoot(real, imag);
                    for (int n = 0; n < j; ++n) {
                        finalRoot = finalRoot - polynomial(finalRoot) / derivative(finalRoot);
                    }

                    // Assign color based on the root and iteration count
                    r = 0, g = 0, b = 0;
                    if (j >= maxIter) {
                        r = g = b = 0; // Non-convergent points: Black
                    } else {
                        if (std::abs(finalRoot - std::complex<float>(1, 0)) < epsilon) {
                            r = 255; g = b = 0; // Red for Root 1
                        } else if (std::abs(finalRoot - std::complex<float>(-0.5, std::sqrt(3) / 2)) < epsilon) {
                            g = 255; r = b = 0; // Green for Root 2
                        } else if (std::abs(finalRoot - std::complex<float>(-0.5, -std::sqrt(3) / 2)) < epsilon) {
                            b = 255; r = g = 0; // Blue for Root 3
                        }

                        // Brightness adjustment based on iteration count
                        float brightness = std::max(0.1f, 1.0f - (float)j / maxIter);
                        r *= brightness;
                        g *= brightness;
                        b *= brightness;
                    }

                    // Store color in the line buffer
                    pixelBuffer[i * SCREEN_WIDTH + k] = (r << 24) | (g << 16) | (b << 8) | SDL_ALPHA_OPAQUE;
                }
            }
    

            // Single-threaded rendering of the buffer
            for (int i = 0; i < SCREEN_HEIGHT; i++) {
                for (int k = 0; k < SCREEN_WIDTH; k++) {
                    Uint32 color = pixelBuffer[i * SCREEN_WIDTH + k];
                    Uint8 r = (color >> 24) & 0xFF;
                    Uint8 g = (color >> 16) & 0xFF;
                    Uint8 b = (color >> 8) & 0xFF;

                    SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
                    SDL_RenderDrawPoint(renderer, k, i);
                }
            }
            SDL_RenderPresent(renderer);

        
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Frame Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " us\n";


            }
            else{
            // Render the frame
            for (int i = 0; i < SCREEN_HEIGHT; i++) {
                for (int k = 0; k < SCREEN_WIDTH; k++) {
                    // Map pixel to complex plane
                    float real = xLowerBound + k * xScale;
                    float imag = yLowerBound + i * yScale;

                    // Compute iteration count
                    j = newton(real, imag, maxIter, epsilon, polynomial, derivative);

                    // Start with initial point and iterate to find the converged root
                    std::complex<float> finalRoot(real, imag);
                    for (int n = 0; n < j; ++n) {
                        finalRoot = finalRoot - polynomial(finalRoot) / derivative(finalRoot);
                    }

                    // Assign color based on the root and iteration count
                    Uint8 r = 0, g = 0, b = 0;

                    if (j >= maxIter) {
                        // Non-convergent points: Black
                        r = g = b = 0;
                    } else {
                        // Assign colors based on the root's proximity
                        if (std::abs(finalRoot - std::complex<float>(1, 0)) < epsilon) {
                            r = 255; // Root 1: Red
                            g = b = 0;
                        } else if (std::abs(finalRoot - std::complex<float>(-0.5, std::sqrt(3) / 2)) < epsilon) {
                            g = 255; // Root 2: Green
                            r = b = 0;
                        } else if (std::abs(finalRoot - std::complex<float>(-0.5, -std::sqrt(3) / 2)) < epsilon) {
                            b = 255; // Root 3: Blue
                            r = g = 0;
                        }

                        // Adjust brightness based on iteration count
                        float brightness = std::max(0.1f, 1.0f - (float)j / maxIter);
                        r *= brightness;
                        g *= brightness;
                        b *= brightness;
                    }

                    // Draw the pixel
                    SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
                    SDL_RenderDrawPoint(renderer, k, i);
                }
                // Update the screen after rendering all pixels
                SDL_RenderPresent(renderer);
                }


            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Frame Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " us\n"; 
        }
    }   

}





    // Quit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}