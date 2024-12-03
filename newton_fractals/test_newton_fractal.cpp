#include <iostream>
#include "newton_fractal.h"

int main() {
    // Polynomial: z^3 - 1 (roots are the three cube roots of unity)
    auto polynomial = [](std::complex<float> z) {
        return z * z * z - std::complex<float>(1.0f);
    };

    // Derivative: 3 * z^2
    auto derivative = [](std::complex<float> z) {
        return 3.0f * z * z;
    };

    // Parameters for testing
    float epsilon = 1e-5;
    int maxIter = 100;

    // Test points
    std::complex<float> testPoints[] = {
        {-1.0f, 1.0f},    
        {-0.5f, 86.6f}, // One of the complex roots
        {7.5f, -0.866f} // Another complex root
    };

    for (const auto& point : testPoints) {
        int iterations = newton(point.real(), point.imag(), maxIter, epsilon, polynomial, derivative);
        std::cout << "Starting point: (" << point.real() << ", " << point.imag() << ")\n";
        if (iterations < maxIter) {
            std::cout << "Converged in " << iterations << " iterations.\n";
        } else {
            std::cout << "Did not converge within " << maxIter << " iterations.\n";
        }
        std::cout << "-----------------------------------\n";
    }

    return 0;
}
