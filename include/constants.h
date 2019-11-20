#pragma once
#include <math.h>

constexpr float PI = 3.1415926535897932384626433f;
constexpr float TWO_PI = PI * 2.0f;
constexpr float INVERSE_PI = 1.0f / PI;
#define BIGFLOAT 1.0e30f
#define HIT_NONE            0
#define HIT_FRONT           1
#define HIT_BACK            2
#define HIT_FRONT_AND_BACK  (HIT_FRONT|HIT_BACK)