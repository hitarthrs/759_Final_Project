#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <chrono>
#include "gpu.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1200
__device__ int maxIter = 100;
//__device__ double bounds[4];

int mandel(float cr, float ci, int maxIter){

    // Temp variables to iterate
    int i;
    float zr = 0;
    float zi = 0;
    float zrsqr = 0;
    float zisqr = 0;

    // Mandelbrot Set determination algorithm
    for(i = 0; i < maxIter; i++) {
        zi = (2 * zi * zr) + ci;
        zr = zrsqr - zisqr + cr;
        zrsqr = zr * zr;
        zisqr = zi * zi;

        if(zrsqr + zisqr > 4.0){
            break;
        }
    }

    return i;
}

void reframe(float zoomRatio,
             int x, int y, int w, int h,
             double *xScale, double *yScale,
             double *xLowerBound, double *xUpperBound,
             double *yLowerBound, double *yUpperBound){

    // Relative mouse position in window
    double xRatio = (double)x/(double)w;
    double yRatio = (double)y/(double)h;

    // Recalculate bounds
    double xLowerBoundNext = *xLowerBound + ((double)zoomRatio * xRatio * (*xUpperBound - *xLowerBound));
    *xUpperBound = *xUpperBound - ((double)zoomRatio * (1.0 - xRatio) * (*xUpperBound - *xLowerBound));
    *xLowerBound = xLowerBoundNext;
    double yLowerBoundNext = *yLowerBound + ((double)zoomRatio * yRatio * (*yUpperBound - *yLowerBound));
    *yUpperBound = *yUpperBound - ((double)zoomRatio * (1.0 - yRatio) * (*yUpperBound - *yLowerBound));
    *yLowerBound = yLowerBoundNext;

    // Recalculate step size
    *xScale = (*xUpperBound - *xLowerBound) / w;
    *yScale = (*yUpperBound - *yLowerBound) / h;

    return;
}

__global__ void kernel(uint32_t* buf, double* bounds){

    // Pixel this thread is responsible for
    int x = blockDim.x * blockIdx.x + threadIdx.x;
    int y = blockDim.y * blockIdx.y + threadIdx.y;

    // Frame overrun guard
    if(x < SCREEN_WIDTH && y < SCREEN_HEIGHT){

        unsigned int pos = SCREEN_WIDTH * y + x;

        // Complex number to calculate
        double cr = bounds[0] + x * bounds[2];
        double ci = bounds[1] + y * bounds[3];

        // Temp variables to iterate
        int i;
        float zr = 0;
        float zi = 0;
        float zrsqr = 0;
        float zisqr = 0;

        // Mandelbrot Set determination algorithm
        for(i = 0; i < maxIter; i++) {
            zi = (2 * zi * zr) + ci;
            zr = zrsqr - zisqr + cr;
            zrsqr = zr * zr;
            zisqr = zi * zi;

            if(zrsqr + zisqr > 4.0){
                break;
            }
        }

        // Store color to buffer
        if(i >= maxIter){
            buf[pos] = 0;
        }else{
            buf[pos] = 256*256*lround(32.0*log2((double)(i+1)));
        }
    }
}

// ------------- MAIN ---------------
int main(){

    // Initialize SDL, window, screen, renderer, and texture

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
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

    // Allocate bounds
    // bounds[0] = xLowerBound
    // bounds[1] = yLowerBound
    // bounds[2] = xScale
    // bounds[3] = yScale
    double* bounds;
    cudaError_t err = cudaMalloc(&bounds, 4*sizeof(double));
    if(err != cudaSuccess){
        printf("Failed to allocate GPU memory.\n");
    }

    // Allocate maxIter
    
    // int* maxIter;
    // err = cudaMalloc(&maxIter, sizeof(int));
    // if(err != cudaSuccess){
    //     printf("Failed to allocate GPU memory.\n");
    // }

    // Allocate frame buffer
    uint32_t* buffer;
    err = cudaMalloc(&buffer, SCREEN_WIDTH*SCREEN_HEIGHT*4);
    if(err != cudaSuccess){
        printf("Failed to allocate GPU memory.\n");
    }

    int xMouse, yMouse;         // Mouse position
    int update = 1;             // Update frame
    float zoomInRatio = 0.5;    // Amount to zoom in by
    float zoomOutRatio = -1.0;  // Amount to zoom out by
    Uint32 buttons;             // buttons to parse
    const Uint8* keys = SDL_GetKeyboardState(NULL);    // keys to parse
    int cudaImp = 0;            // use cuda implementation
    int j;                      // iterator in CPU implementation

    // Default bounds
    double* boundsHost = (double*)malloc(4*sizeof(double));
    double xLowerBound = -2.21;
    double xUpperBound = 1.63;
    double yLowerBound = -1.2;
    double yUpperBound = 1.2;
    double xScale = (xUpperBound - xLowerBound) / SCREEN_WIDTH;
    double yScale = (yUpperBound - yLowerBound) / SCREEN_HEIGHT;

    // Fill bounds and send to device
    boundsHost[0] = xLowerBound;
    boundsHost[1] = yLowerBound;
    boundsHost[2] = xScale;
    boundsHost[3] = yScale;
    cudaMemcpy(bounds, boundsHost, 4*sizeof(double), cudaMemcpyHostToDevice);

    // Send maxIter to device
    int maxIterHost = 100;
    // *maxIterHost = 100;
    // cudaMemcpy(maxIter, maxIterHost, sizeof(int), cudaMemcpyHostToDevice);
    

    dim3 tpb; // Threads per block
    tpb.x = 32;
    tpb.y = 32;

    dim3 bpg; // Blocks per grid
    bpg.x = (SCREEN_WIDTH + tpb.x - 1)/tpb.x;
    bpg.y = (SCREEN_HEIGHT + tpb.y - 1)/tpb.y;

    printf("\nLaunching CPU implementation first,\n");
    printf("Press 'S' to switch between CPU and CUDA implementations.\n");

    bool running = true;
    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT: // Exit loop and clean up
                    running = false;
                    break;

                case SDL_MOUSEBUTTONDOWN: // Mouse button pressed

                    buttons = SDL_GetMouseState(&xMouse, &yMouse);
                    if((buttons & SDL_BUTTON_LMASK) != 0){

                        // Left click: zoom in
                        update = 1;
                        reframe(zoomInRatio,
                                xMouse, yMouse, SCREEN_WIDTH, SCREEN_HEIGHT,
                                &xScale, &yScale,
                                &xLowerBound, &xUpperBound,
                                &yLowerBound, &yUpperBound);

                    }else if((buttons & SDL_BUTTON_RMASK) != 0){

                        // Right click: zoom out
                        update = 1;
                        reframe(zoomOutRatio,
                                xMouse, yMouse, SCREEN_WIDTH, SCREEN_HEIGHT,
                                &xScale, &yScale,
                                &xLowerBound, &xUpperBound,
                                &yLowerBound, &yUpperBound);

                    }

                    // Update bounds on device
                    boundsHost[0] = xLowerBound;
                    boundsHost[1] = yLowerBound;
                    boundsHost[2] = xScale;
                    boundsHost[3] = yScale;
                    cudaMemcpy(bounds, boundsHost, 4*sizeof(double), cudaMemcpyHostToDevice);

                    break;

                case SDL_KEYDOWN:
                    SDL_GetKeyboardState(NULL);
                    if(keys[SDL_Scancode::SDL_SCANCODE_S]){
                        
                        update = 1;
                        if(cudaImp){
                            printf("\nSwitching to CPU implementation (Patience!)...\n");
                            cudaImp = 0;
                        }else{
                            printf("\nSwitching to CUDA implementation...\n");
                            cudaImp = 1;
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        if(update){
            update = 0;

            // Time Frame-time
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

            if(cudaImp){ // Use CUDA

                // Render and retrieve frame
                SDL_LockSurface(screen);
                kernel<<<bpg, tpb>>>(buffer, bounds);
                cudaMemcpy(screen->pixels, buffer, SCREEN_WIDTH*SCREEN_HEIGHT*4, cudaMemcpyDeviceToHost);
                SDL_UnlockSurface(screen);

                // Update Screen
                SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);

            }else{ // Use CPU

                // Iterate through all pixels
                for(int i = 0; i < SCREEN_HEIGHT; i++){
                    for(int k = 0; k < SCREEN_WIDTH; k++){

                        // Calculate if in Mandelbrot Set, set color accordingly
                        j = mandel(xLowerBound + (xScale * k), yLowerBound + (yScale * i), maxIterHost);
                        if(j >= maxIterHost){
                            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                        }else{
                            SDL_SetRenderDrawColor(renderer, 32*log2(j+1), 0, 0, SDL_ALPHA_OPAQUE);
                        }
                        SDL_RenderDrawPoint(renderer, k, i);
                    }
                    SDL_RenderPresent(renderer); // Update screen after each line to show progress
                }
            }

            // Print Frame-time
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            printf("Frame Time: %I64i us\n", std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count());
        }
    }

    // Free host and device memory
    cudaFree(bounds);
    //cudaFree(maxIter);
    cudaFree(buffer);
    free(boundsHost);
    //free(maxIterHost);

    // Quit
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}