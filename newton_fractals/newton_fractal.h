#ifndef NEWTON_FRACTAL_H
#define NEWTON_FRACTAL_H

#include <complex>
#include <functional>

#define MAX_ITERATIONS 100
#define EPSILON 1e-5

// Function to compute the Newton fractal iteration count
// Parameters:
//   - real: The real part of the initial guess
//   - imag: The imaginary part of the initial guess
//   - maxIter: Maximum number of iterations allowed
//   - epsilon: Convergence threshold for determining a root
//   - polynomial: The polynomial function to solve
//   - derivative: The derivative of the polynomial function
int newton(std::complex<float>& z);

#endif // NEWTON_FRACTAL_H