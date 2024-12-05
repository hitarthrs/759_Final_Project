#include "lyapunov_fractal.h"
#include <cmath>

// Computes the Lyapunov exponent
float computeLyapunov(const std::string& sequence, float a, float b) {
    size_t seqLength = sequence.size();
    if (seqLength == 0) return -1.0f; // Invalid sequence

    float x = 0.5f; // Initial condition
    float lyapunovExponent = 0.0f;

    for (int i = 0; i < 6000; ++i) { // MAX_ITERATIONS is set to 6000
        float r = (sequence[i % seqLength] == 'A') ? a : b;

        x = r * x * (1.0f - x);
        if (x <= 0.0f || x >= 1.0f) return -1.0f;

        float derivative = std::abs(r * (1.0f - 2.0f * x));
        if (derivative < 1e-6f) return -1.0f; // Avoid log(0)

        lyapunovExponent += std::log(derivative);
    }

    return lyapunovExponent / 6000; // MAX_ITERATIONS
}

// Maps a Lyapunov exponent value to a color (RGBA)
uint32_t mapLyapunovToColor(float lyapunov) {
    if (lyapunov < 0) {
        // Dark blue color for divergent points
        uint8_t intensity = static_cast<uint8_t>(128 * std::max(0.0f, 1.0f - std::abs(lyapunov / 1.5f)));
        return (intensity << 16) | (intensity << 8) | 64 | 0xFF; // Dark blue
    } else {
        // Dark gold color for stable points
        return 0xB8860BFF; // Dark gold (#B8860B)
    }
}
