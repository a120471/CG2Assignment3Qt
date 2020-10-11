#pragma once

#include <ctime>

namespace ray_tracing {

const float MYINFINITE = 999999.f;
const float MYEPSILON = 2e-4;
const float COLOR_INTENSITY_THRES = 2e4f;
const float UNIT_SAMPLE_COLOR = COLOR_INTENSITY_THRES / 10.f;
const float NTHIDX = 0.85f;
const uint32_t MYTHREADNUM = 8u;

extern bool hasHDRLighting;

}
