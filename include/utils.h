#pragma once

#include <math.h>
#include "cyVector.h"
#include <vector>
#include <string>

using namespace cy;

#define RANDOM_THRESHOLD 0.05f 

Vec3f RandomInUnitSphere();

Vec2f NonUniformRandomPointInCircle(float radius);

Vec2f RandomPointInCircle(float radius);

void BranchlessONB(const Vec3f& n, Vec3f& b1, Vec3f& b2);

void CommonOrthonormalBasis(const Vec3f& n, Vec3f& b1, Vec3f& b2);

Vec3f UniformRandomPointOnHemiSphere();

Vec3f CosineWeightedRandomPointOnHemiSphere();

Vec3f ImportanceSampleGGX(float roughness, float& probability);

int MIS2(float p1, float p2);
int MIS3(float p1, float p2, float p3);

float LightFallOffFactor(const Vec3f& p1, const Vec3f& p2);
float LightFallOffFactor(float distance);

Vec2f UniformSampleTriangle();

int RandomIndexElementInList(int size);

float RandomRange(float left, float right);

class CDF 
{
public:
	CDF();

	void Init();

	// rand from 0 to tatal, return the correspond id
	int Sample() const;

	void Add(float val);

	float total;
	std::vector<float> cd;
};

Vec3f ParseVec3f(std::string& str);