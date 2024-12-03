#ifndef TIME_FRAMING_H
#define TIME_FRAMING_H

#include <functional>
#include <complex>

// Function to time the computation of 5 zoomed frames
double timeZoomedFrames(float zoomRatio, 
                        int xMouse, int yMouse, 
                        int screenWidth, int screenHeight,
                        float initialXScale, float initialYScale, 
                        float initialXLower, float initialXUpper, 
                        float initialYLower, float initialYUpper, 
                        int maxIter, float epsilon, 
                        const std::function<std::complex<float>(std::complex<float>)>& polynomial, 
                        const std::function<std::complex<float>(std::complex<float>)>& derivative);

#endif // TIME_FRAMING_H
