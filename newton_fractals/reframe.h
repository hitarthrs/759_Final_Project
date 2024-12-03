#ifndef REFRAME_H
#define REFRAME_H

#include <iostream>

// Function to adjust the viewing window during zoom operations
// Parameters:
// - zoomRatio: The factor by which to zoom in (>1) or out (<1)
// - x, y: The mouse position in the window (pixels)
// - w, h: The width and height of the window (pixels)
// - xScale, yScale: Pointers to the X and Y axis step sizes (scaling factors)
// - xLowerBound, xUpperBound: Pointers to the current lower and upper bounds of the X-axis
// - yLowerBound, yUpperBound: Pointers to the current lower and upper bounds of the Y-axis
void reframe(float zoomRatio,
             int x, int y, int w, int h,
             float* xScale, float* yScale,
             float* xLowerBound, float* xUpperBound,
             float* yLowerBound, float* yUpperBound);

#endif // REFRAME_H
