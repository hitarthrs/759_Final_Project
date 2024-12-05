#ifndef COMPUTE_LYAPUNOV_H
#define COMPUTE_LYAPUNOV_H

#include <string>
#include <cstdint>

// Computes the Lyapunov exponent for a given sequence, and parameters (a, b)
float computeLyapunov(const std::string& sequence, float a, float b);

// Maps a Lyapunov exponent value to a color (RGBA format)
uint32_t mapLyapunovToColor(float lyapunov);

#endif // COMPUTE_LYAPUNOV_H
