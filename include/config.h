#pragma once
#include <string>

// Sample Count
constexpr int MinPixelSampleCount = 8;
constexpr int MaxPixelSampleCount = 16;
// Lighting
constexpr bool LightFallOff = true;
constexpr bool IrradianceCache = true;
constexpr int IrradianceGISampleCount = 16;
constexpr unsigned int IndirectLightSampleCount = 16;
// Shadow
constexpr int MinShadowSampleCount = 4;
constexpr int MaxShadowSampleCount = 8;
constexpr float ShadowTolerance = 0.01f;
// Refraction/Reflection Bounce Count
constexpr int RefractionBounceCount = 4;
constexpr int IndirectLightBounceCount = 1;
constexpr char* ScenePath = "assets/project11.xml";