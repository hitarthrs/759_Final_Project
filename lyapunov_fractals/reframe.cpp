#include "reframe.h"

void reframeLyapunov(
    float zoomRatio,         // Amount to zoom (positive for zoom in, negative for zoom out)
    int x, int y,            // Mouse position
    int screenWidth, int screenHeight, // Screen dimensions
    float* aMin, float* aMax,// Bounds for parameter A
    float* bMin, float* bMax,// Bounds for parameter B
    float* aScale, float* bScale // Scaling factors
) {
    // Calculate relative mouse position in the window
    float xRatio = static_cast<float>(x) / screenWidth;
    float yRatio = static_cast<float>(y) / screenHeight;

    // Update bounds for parameter A (horizontal axis)
    float deltaA = zoomRatio * (*aMax - *aMin);
    *aMin += deltaA * xRatio;
    *aMax -= deltaA * (1.0f - xRatio);

    // Update bounds for parameter B (vertical axis)
    float deltaB = zoomRatio * (*bMax - *bMin);
    *bMin += deltaB * yRatio;
    *bMax -= deltaB * (1.0f - yRatio);

    // Recalculate scale factors
    *aScale = (*aMax - *aMin) / screenWidth;
    *bScale = (*bMax - *bMin) / screenHeight;
}
