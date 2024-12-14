#ifndef RENDER_CUDA_LYAPUNOV_H
#define RENDER_CUDA_LYAPUNOV_H

#include <vector>
#include <cstdint>
#include <string>

void renderCuda(std::vector<uint32_t>& pixelBuffer, int screenWidth, int screenHeight, float aMin, float bMin, float aScale, float bScale, std::string sequence);

#endif // REFRAME_H

