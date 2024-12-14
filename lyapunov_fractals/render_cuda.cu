#include "render_cuda.h"
#include <cuda/std/cmath>
#include <iostream>
#include <chrono>

// Computes the Lyapunov exponent
__device__ float computeLyapunovDevice(char* sequence, int seqLength, float a, float b) {
    if (seqLength == 0) return -1.0f; // Invalid sequence

    float x = 0.5f; // Initial condition
    float lyapunovExponent = 0.0f;

    for (int i = 0; i < 6000; ++i) { // MAX_ITERATIONS is set to 6000
        float r = (sequence[i % seqLength] == 'A') ? a : b;

        x = r * x * (1.0f - x);
        if (x <= 0.0f || x >= 1.0f) return -1.0f;

        float derivative = cuda::std::abs(r * (1.0f - 2.0f * x));
        if (derivative < 1e-6f) return -1.0f; // Avoid log(0)

        lyapunovExponent += cuda::std::log(derivative);
    }

    return lyapunovExponent / 6000; // MAX_ITERATIONS
}

// Maps a Lyapunov exponent value to a color (RGBA)
__device__ uint32_t mapLyapunovToColorDevice(float lyapunov) {
    if (lyapunov < 0) {
        // Dark blue color for divergent points
        uint8_t intensity = static_cast<uint8_t>(128 * max(0.0f, 1.0f - cuda::std::abs(lyapunov / 1.5f)));
        return (intensity << 16) | (intensity << 8) | 64 | 0xFF; // Dark blue
    } else {
        // Dark gold color for stable points
        return 0xB8860BFF; // Dark gold (#B8860B)
    }
}

__global__ void renderCudaKernel(uint32_t *d_pixelBuffer, int screenWidth, int screenHeight, float aMin, float bMin, float aScale, float bScale, char* d_sequence, int seqLength) {

    int x = threadIdx.x + blockIdx.x * blockDim.x;
    int y = threadIdx.y + blockIdx.y * blockDim.y;

    float a = aMin + x * aScale;
    float b = bMin + y * bScale;

    if(x < screenWidth && y < screenHeight) {

        // Compute Lyapunov exponent
        float lyapunov = computeLyapunovDevice(d_sequence, seqLength, a, b);

        // Map Lyapunov exponent to color
        d_pixelBuffer[y * screenWidth + x] = mapLyapunovToColorDevice(lyapunov);
    }
}

void renderCuda(std::vector<uint32_t>& pixelBuffer, int screenWidth, int screenHeight, float aMin, float bMin, float aScale, float bScale, std::string sequence) {

    uint32_t *d_pixelBuffer;
    cudaMalloc(&d_pixelBuffer, sizeof(uint32_t) * screenHeight * screenWidth);

    char* d_sequence;
    int seqLength = sequence.length();
    cudaMalloc(&d_sequence, seqLength);
    cudaMemcpy(d_sequence, &sequence.at(0), seqLength, cudaMemcpyHostToDevice);

    dim3 threadsPerBlock(16, 16, 1);
    dim3 blocksPerGrid((screenWidth + threadsPerBlock.x - 1) / threadsPerBlock.x,
                       (screenHeight + threadsPerBlock.y - 1) / threadsPerBlock.y,
                       1);

    // Start timer
    auto startTime = std::chrono::high_resolution_clock::now();

    renderCudaKernel<<<blocksPerGrid, threadsPerBlock>>>(d_pixelBuffer, screenWidth, screenHeight, aMin, bMin, aScale, bScale, d_sequence, seqLength);
    cudaDeviceSynchronize();

    // Stop timer
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Fractal computed in " << duration << " ms\n";

    cudaMemcpy(&pixelBuffer.at(0), d_pixelBuffer, sizeof(uint32_t) * screenHeight * screenWidth, cudaMemcpyDeviceToHost);
    cudaFree(d_pixelBuffer);
    cudaFree(d_sequence);
}
