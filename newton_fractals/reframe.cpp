#include "reframe.h"

void reframe(float zoomRatio,
             int x, int y, int w, int h,
             double* xScale, double* yScale,
             double* xLowerBound, double* xUpperBound,
             double* yLowerBound, double* yUpperBound) {
    // Relative mouse position in window
    double xRatio = (double)x / (double)w;
    double yRatio = (double)y / (double)h;

    // Recalculate bounds
    double xLowerBoundNext = *xLowerBound + ((double)zoomRatio * xRatio * (*xUpperBound - *xLowerBound));
    *xUpperBound = *xUpperBound - ((double)zoomRatio * (1.0 - xRatio) * (*xUpperBound - *xLowerBound));
    *xLowerBound = xLowerBoundNext;

    double yLowerBoundNext = *yLowerBound + ((double)zoomRatio * yRatio * (*yUpperBound - *yLowerBound));
    *yUpperBound = *yUpperBound - ((double)zoomRatio * (1.0 - yRatio) * (*yUpperBound - *yLowerBound));
    *yLowerBound = yLowerBoundNext;

    // Recalculate step size
    *xScale = (*xUpperBound - *xLowerBound) / w;
    *yScale = (*yUpperBound - *yLowerBound) / h;

    return;
}
