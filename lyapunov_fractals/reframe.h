#ifndef REFRAME_H
#define REFRAME_H

#include <iostream>

// Function to adjust the parameter space during zoom operations for Lyapunov fractals.
// Parameters:
// - zoomRatio: The factor by which to zoom in (>1) or out (<1).
// - x, y: The mouse position in the window (pixels).
// - screenWidth, screenHeight: The width and height of the window (pixels).
// - aScale, bScale: Pointers to the step sizes (scaling factors) for A and B.
// - aMin, aMax: Pointers to the current lower and upper bounds of parameter A.
// - bMin, bMax: Pointers to the current lower and upper bounds of parameter B.
void reframeLyapunov(
    float zoomRatio,
    int x, int y,
    int screenWidth, int screenHeight,
    float* aMin, float* aMax,
    float* bMin, float* bMax,
    float* aScale, float* bScale
);
#endif // REFRAME_H
