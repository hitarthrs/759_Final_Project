#include "time_framing.h"
#include "reframe.h"
#include <chrono>
#include "newton_fractal.h"
#include <omp.h>

int chunk_size = 1200; // Example chunk size

double timeZoomedFrames(float zoomRatio, 
                        int xMouse, int yMouse, 
                        int screenWidth, int screenHeight,
                        float initialXScale, float initialYScale, 
                        float initialXLower, float initialXUpper, 
                        float initialYLower, float initialYUpper, 
                        int maxIter, float epsilon, 
                        const std::function<std::complex<float>(std::complex<float>)>& polynomial, 
                        const std::function<std::complex<float>(std::complex<float>)>& derivative) {
    // Initialize bounds and scales
    float xScale = initialXScale;
    float yScale = initialYScale;
    float xLowerBound = initialXLower;
    float xUpperBound = initialXUpper;
    float yLowerBound = initialYLower;
    float yUpperBound = initialYUpper;

    // Timer start
    auto start = std::chrono::steady_clock::now();

    // Compute 5 zoomed frames
    for (int frame = 0; frame < 5; frame++) {
        // Recalculate bounds using reframe
        reframe(zoomRatio, xMouse, yMouse, screenWidth, screenHeight,
                &xScale, &yScale, &xLowerBound, &xUpperBound, &yLowerBound, &yUpperBound);

        // Compute fractal data
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < screenHeight; i++) {
            for (int k = 0; k < screenWidth; k++) {
                // Map pixel to complex plane
                float real = xLowerBound + k * xScale;
                float imag = yLowerBound + i * yScale;

                // Compute fractal iterations
                int j = newton(real, imag, maxIter, epsilon, polynomial, derivative);
                std::complex<float> finalRoot(real, imag);
                for (int n = 0; n < j; ++n) {
                    finalRoot = finalRoot - polynomial(finalRoot) / derivative(finalRoot);
                }

                // No rendering; only compute values
            }
        }
    }

    // Timer end
    auto end = std::chrono::steady_clock::now();

    // Calculate and return total time
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}


int main() {
    // Example polynomial and derivative for z^3 - 1
    auto polynomial = [](std::complex<float> z) {
        return z * z * z - std::complex<float>(1.0f, 0.0f);
    };
    auto derivative = [](std::complex<float> z) {
        return 3.0f * z * z;
    };

    omp_set_num_threads(19);
    // Time the computation of 5 zoomed frames
    double elapsedTime = timeZoomedFrames(0.5f, 640, 360, 1900, 1200, 
                                          0.01f, 0.01f, -2.0f, 2.0f, -1.5f, 1.5f, 
                                          100, 1e-5f, polynomial, derivative);

    std::cout << "Time taken for 5 zoomed frames: " << elapsedTime << " ms" << std::endl;
    return 0;
}