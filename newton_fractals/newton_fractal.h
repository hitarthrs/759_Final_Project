#ifndef NEWTON_FRACTAL_H
#define NEWTON_FRACTAL_H

#include <complex>
#include <functional>

// Function to compute the Newton fractal iteration count
int newton(float real, float imag, int maxIter, float epsilon,
           const std::function<std::complex<float>(std::complex<float>)>& polynomial,
           const std::function<std::complex<float>(std::complex<float>)>& derivative) {
    // Initialize the complex number z as (real, imag)
    std::complex<float> z(real, imag);

    // Newton's method iteration
    int i;
    for (i = 0; i < maxIter; i++) {
        std::complex<float> f = polynomial(z);
        std::complex<float> f_prime = derivative(z);

        // Avoid division by zero
        if (std::abs(f_prime) < epsilon) {
            break;
        }

        // Update z using Newton's formula: z = z - f(z) / f'(z)
        z -= f / f_prime;

        // Check for convergence to a root
        if (std::abs(f) < epsilon) {
            break;
        }
    }

    return i; // Return the number of iterations
}

#endif // NEWTON_FRACTAL_H
