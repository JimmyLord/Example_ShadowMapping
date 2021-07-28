#pragma once

#include <math.h>

#define PI 3.14159265358979323846f

namespace fw {

void OutputMessage(const char* message, ...);
char* LoadCompleteFile(const char* filename, long* length);
double GetSystemTime();
double GetSystemTimeSinceGameStart();

} // namespace fw

const float FEQUALEPSILON = 0.00001f;

inline bool fequal(const float a, const float b, const float epsilon = FEQUALEPSILON)
{
    return fabs(a - b) <= epsilon;
}
