#include "newton_fractal.h"
#include <complex>
#include <cmath>

// Polynomial and derivative for Newton's method (example: z^3 - 1)
static inline std::complex<float> polynomial(std::complex<float> z) {
    return z * z * z - std::complex<float>(1.0f, 0.0f);
}

static inline std::complex<float> derivative(std::complex<float> z) {
    return 3.0f * z * z;
}

int newton(std::complex<float>& z) {
    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        const std::complex<float> fz = polynomial(z);
        const std::complex<float> fzPrime = derivative(z);

        // Avoid division by a very small number (derivative close to zero)
        if (std::abs(fzPrime) < EPSILON)
            return MAX_ITERATIONS;

        const std::complex<float> nextZ = z - fz / fzPrime;

        if (std::abs(nextZ - z) < EPSILON) // Converged to a root
            return i;

        z = nextZ;
    }
    return MAX_ITERATIONS; // Did not converge within the maximum number of iterations
}