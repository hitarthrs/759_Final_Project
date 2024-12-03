#include "reframe.h"

void reframe(float zoomRatio,
             int x, int y, int w, int h,
             float* xScale, float* yScale,
             float* xLowerBound, float* xUpperBound,
             float* yLowerBound, float* yUpperBound) {
    // Calculate relative mouse position in the window
    float xRatio = static_cast<float>(x) / w;
    float yRatio = static_cast<float>(y) / h;

    // Update X-axis bounds
    float deltaX = zoomRatio * (*xUpperBound - *xLowerBound);
    *xLowerBound += deltaX * xRatio;
    *xUpperBound -= deltaX * (1.0f - xRatio);

    // Update Y-axis bounds
    float deltaY = zoomRatio * (*yUpperBound - *yLowerBound);
    *yLowerBound += deltaY * yRatio;
    *yUpperBound -= deltaY * (1.0f - yRatio);

    // Update scale factors
    *xScale = (*xUpperBound - *xLowerBound) / w;
    *yScale = (*yUpperBound - *yLowerBound) / h;
}
