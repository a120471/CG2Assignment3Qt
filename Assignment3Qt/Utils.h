#pragma once

#include <vector>
#include <cmath>
#include "rayTracingCamera.h"
#include <ctime>

#ifndef INFINITE
#define INFINITE 999999
#endif // !INFINITE

#ifndef EPSILON
#define EPSILON 2e-4
#endif // !EPSILON

#ifndef COLORINTENSITYTHRES
#define COLORINTENSITYTHRES 4e4f
#endif // !COLORINTENSITYTHRES

#ifndef UNITCOLORINTENSITY
#define UNITCOLORINTENSITY (COLORINTENSITYTHRES / 10.0f)
#endif // !UNITCOLORINTENSITY

#ifndef NTHIDX
#define NTHIDX 85 / 100
#endif // !NTHIDX


template<typename T> void safe_delete(T*& a)
{
	delete a;
	a = NULL;
}

template <typename T>
void inline MySwap(T &t1, T &t2)
{
	T tmp = t1;
	t1 = t2;
	t2 = tmp;
};

static void MergeBoundingBox(glm::vec3 &A, glm::vec3 &B, glm::vec3 A1, glm::vec3 B1, glm::vec3 A2, glm::vec3 B2)
{
	A[0] = glm::min(A1[0], A2[0]);
	A[1] = glm::min(A1[1], A2[1]);
	A[2] = glm::min(A1[2], A2[2]);

	B[0] = glm::max(B1[0], B2[0]);
	B[1] = glm::max(B1[1], B2[1]);
	B[2] = glm::max(B1[2], B2[2]);
}

static bool RayHitAABB(RayClass *ray, glm::vec3 A, glm::vec3 B)
{
	float dx = abs(ray->direction.x);
	float dy = abs(ray->direction.y);
	float dz = abs(ray->direction.z);

	glm::vec3 A_RS = A - ray->sPoint;
	glm::vec3 B_RS = B - ray->sPoint;

	if (dx < EPSILON && (A_RS.x > EPSILON || B_RS.x < -EPSILON))
		return false;
	if (dy < EPSILON && (A_RS.y > EPSILON || B_RS.y < -EPSILON))
		return false;
	if (dz < EPSILON && (A_RS.z > EPSILON || B_RS.z < -EPSILON))
		return false;
	
	float maxTMin = -INFINITE, minTMax = INFINITE, maxTMax = -INFINITE;
	
	if (dx > EPSILON) 
	{
		float t1 = A_RS.x / ray->direction.x;
		float t2 = B_RS.x / ray->direction.x;
		if (t1 > t2)
			MySwap(t1, t2);

		if (maxTMin < t1) maxTMin = t1;
		if (minTMax > t2) minTMax = t2;
		if (maxTMax < t2) maxTMax = t2;
	}	
	if (dy > EPSILON)
	{
		float t1 = A_RS.y / ray->direction.y;
		float t2 = B_RS.y / ray->direction.y;
		if (t1 > t2)
			MySwap(t1, t2);

		if (maxTMin < t1) maxTMin = t1;
		if (minTMax > t2) minTMax = t2;
		if (maxTMax < t2) maxTMax = t2;
	}
	if (dz > EPSILON)
	{
		float t1 = A_RS.z / ray->direction.z;
		float t2 = B_RS.z / ray->direction.z;
		if (t1 > t2)
			MySwap(t1, t2);

		if (maxTMin < t1) maxTMin = t1;
		if (minTMax > t2) minTMax = t2;
		if (maxTMax < t2) maxTMax = t2;
	}

	if (maxTMin < minTMax + EPSILON && maxTMax > EPSILON)
		return true;
	else
		return false;
}

// this function is inefficient and not precise
static void BestCandidateAlgorithm(std::vector<glm::vec2> &point, int num, float w, float h)
{
	// candidate num is 2 here
	int candidateNum = 2;

	point.clear();
	if (num == 0)
		return;

	srand(time(0));
	point.push_back(glm::vec2(rand() * w / RAND_MAX - w / 2, rand() * h / RAND_MAX - h / 2));
	for (int i = 1; i < num; i++)
	{
		float maxDis = 0;
		glm::vec2 curCandidate;
		for (int j = 0; j < candidateNum; j++)
		{
			glm::vec2 tmpCandidate(rand() * w / RAND_MAX - w / 2, rand() * h / RAND_MAX - h / 2);
			for (std::vector<glm::vec2>::iterator k = point.begin(); k != point.end(); k++)
			{
				float tmpDis = length(*k - tmpCandidate);
				if (maxDis < tmpDis)
				{
					maxDis = tmpDis;
					curCandidate = tmpCandidate;
				}
			}
		}
		point.push_back(curCandidate);
	}
}