#ifndef RENDER_CUDA_H
#define RENDER_CUDA_H

#include <vector>
#include <cstdint>

void renderCuda(std::vector<uint32_t>& pixelBuffer, int screenWidth, int screenHeight, float xLowerBound, float yLowerBound, float xScale, float yScale);

#endif // REFRAME_H
