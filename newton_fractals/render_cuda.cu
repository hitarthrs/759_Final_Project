#include "render_cuda.h"
#include "newton_fractal.h"
#include <complex>
#include <cuda/std/complex>
#include <chrono>
#include <iostream>

// Polynomial and derivative for Newton's method (example: z^3 - 1)
__device__ static inline cuda::std::complex<float> polynomial(cuda::std::complex<float> z) {
    return z * z * z - cuda::std::complex<float>(1.0f, 0.0f);
}

__device__ static inline cuda::std::complex<float> derivative(cuda::std::complex<float> z) {
    return 3.0f * z * z;
}

__device__ int newtonGpu(cuda::std::complex<float>& z) {
    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        const cuda::std::complex<float> fz = polynomial(z);
        const cuda::std::complex<float> fzPrime = derivative(z);

        // Avoid division by a very small number (derivative close to zero)
        if (cuda::std::abs(fzPrime) < EPSILON)
            return MAX_ITERATIONS;

        const cuda::std::complex<float> nextZ = z - fz / fzPrime;

        if (cuda::std::abs(nextZ - z) < EPSILON) // Converged to a root
            return i;

        z = nextZ;
    }
    return MAX_ITERATIONS; // Did not converge within the maximum number of iterations
}

__global__ void renderCudaKernel(uint32_t *d_pixelBuffer, int screenWidth, int screenHeight, float xLowerBound, float yLowerBound, float xScale, float yScale) {

    int i = threadIdx.y + blockIdx.y * blockDim.y;
    int k = threadIdx.x + blockIdx.x * blockDim.x;

    if(i < screenHeight && k < screenWidth) {

        // Map pixel to complex plane
        cuda::std::complex<float> z(xLowerBound + k * xScale, yLowerBound + i * yScale);

        // Compute Newton
        const int j = newtonGpu(z);

        // Assign color based on the root and iteration count
        uint32_t brightness = 255.0f * max(0.1f, 1.0f - (float)j / MAX_ITERATIONS);

        if (j < MAX_ITERATIONS) {
            
            if (cuda::std::abs(z - cuda::std::complex<float>(1, 0)) < EPSILON)
                brightness <<= 24; // Red for Root 1
            
            else if (cuda::std::abs(z - cuda::std::complex<float>(-0.5, sqrt(3.0) / 2)) < EPSILON)
                brightness <<= 16; // Green for Root 2
            
            else if (cuda::std::abs(z - cuda::std::complex<float>(-0.5, -sqrt(3.0) / 2)) < EPSILON)
                brightness <<= 8; // Blue for Root 3
        }

        // Store color in the line buffer
        d_pixelBuffer[i * screenWidth + k] = brightness | 0x000000FF;
    }
}

void renderCuda(std::vector<uint32_t>& pixelBuffer, int screenWidth, int screenHeight, float xLowerBound, float yLowerBound, float xScale, float yScale) {
    
    uint32_t *d_pixelBuffer;
    cudaMalloc(&d_pixelBuffer, sizeof(uint32_t) * screenHeight * screenWidth);

    dim3 threadsPerBlock(16, 16, 1);
    dim3 blocksPerGrid((screenWidth + threadsPerBlock.x - 1) / threadsPerBlock.x,
                       (screenHeight + threadsPerBlock.y - 1) / threadsPerBlock.y,
                       1);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    renderCudaKernel<<<blocksPerGrid, threadsPerBlock>>>(d_pixelBuffer, screenWidth, screenHeight, xLowerBound, yLowerBound, xScale, yScale);
    cudaDeviceSynchronize();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Frame Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " us\n";

    cudaMemcpy(&pixelBuffer.at(0), d_pixelBuffer, sizeof(uint32_t) * screenHeight * screenWidth, cudaMemcpyDeviceToHost);
    cudaFree(d_pixelBuffer);
}
