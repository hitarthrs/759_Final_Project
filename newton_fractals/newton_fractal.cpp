#include "newton_fractal.h"
#include <complex>
#include <cmath>

int newton(float real, float imag, int maxIter, float epsilon,
           const std::function<std::complex<float>(std::complex<float>)>& polynomial,
           const std::function<std::complex<float>(std::complex<float>)>& derivative) {
    std::complex<float> z(real, imag);
    for (int i = 0; i < maxIter; ++i) {
        std::complex<float> fz = polynomial(z);
        std::complex<float> fzPrime = derivative(z);

        if (std::abs(fzPrime) < epsilon) {
            // Avoid division by a very small number (derivative close to zero)
            return maxIter;
        }

        std::complex<float> nextZ = z - fz / fzPrime;

        if (std::abs(nextZ - z) < epsilon) {
            // Converged to a root
            return i;
        }

        z = nextZ;
    }
    return maxIter; // Did not converge within the maximum number of iterations
}