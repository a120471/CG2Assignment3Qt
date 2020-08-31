#pragma once

// #include <vector>
// #include <cmath>
// #include "rayTracingCamera.h"
// #include <ctime>

// #ifndef MYINFINITE
// #define MYINFINITE 999999
// #endif // !MYINFINITE

// #ifndef MYEPSILON
// #define MYEPSILON 2e-4
// #endif // !MYEPSILON

// #ifndef COLORINTENSITYTHRES
// #define COLORINTENSITYTHRES 2e4f
// #endif // !COLORINTENSITYTHRES

// #ifndef UNITAREASAMPLECOLOR
// #define UNITAREASAMPLECOLOR (COLORINTENSITYTHRES / 10.0f)
// #endif // !UNITAREASAMPLECOLOR

// #ifndef NTHIDX
// #define NTHIDX 85 / 100
// #endif // !NTHIDX

// #ifndef MYTHREADNUM
// #define MYTHREADNUM 8
// #endif // !MYTHREADNUM

// extern bool hasHDRLighting;

namespace ray_tracing {

template<typename T> void SafeDelete(T *&p) {
	delete p;
	p = nullptr;
}

// template <typename T>
// void inline MySwap(T &t1, T &t2)
// {
// 	T tmp = t1;
// 	t1 = t2;
// 	t2 = tmp;
// };

// static void MergeBoundingBox(glm::vec3 &A, glm::vec3 &B, glm::vec3 A1, glm::vec3 B1, glm::vec3 A2, glm::vec3 B2)
// {
// 	A[0] = glm::min(A1[0], A2[0]);
// 	A[1] = glm::min(A1[1], A2[1]);
// 	A[2] = glm::min(A1[2], A2[2]);

// 	B[0] = glm::max(B1[0], B2[0]);
// 	B[1] = glm::max(B1[1], B2[1]);
// 	B[2] = glm::max(B1[2], B2[2]);
// }

// static bool RayHitAABB(RayClass *ray, glm::vec3 A, glm::vec3 B)
// {
// 	float dx = abs(ray->direction.x);
// 	float dy = abs(ray->direction.y);
// 	float dz = abs(ray->direction.z);

// 	glm::vec3 A_RS = A - ray->sPoint;
// 	glm::vec3 B_RS = B - ray->sPoint;

// 	if (dx < MYEPSILON && (A_RS.x > MYEPSILON || B_RS.x < -MYEPSILON))
// 		return false;
// 	if (dy < MYEPSILON && (A_RS.y > MYEPSILON || B_RS.y < -MYEPSILON))
// 		return false;
// 	if (dz < MYEPSILON && (A_RS.z > MYEPSILON || B_RS.z < -MYEPSILON))
// 		return false;

// 	float maxTMin = -MYINFINITE, minTMax = MYINFINITE, maxTMax = -MYINFINITE;

// 	if (dx > MYEPSILON)
// 	{
// 		float t1 = A_RS.x / ray->direction.x;
// 		float t2 = B_RS.x / ray->direction.x;
// 		if (t1 > t2)
// 			MySwap(t1, t2);

// 		if (maxTMin < t1) maxTMin = t1;
// 		if (minTMax > t2) minTMax = t2;
// 		if (maxTMax < t2) maxTMax = t2;
// 	}
// 	if (dy > MYEPSILON)
// 	{
// 		float t1 = A_RS.y / ray->direction.y;
// 		float t2 = B_RS.y / ray->direction.y;
// 		if (t1 > t2)
// 			MySwap(t1, t2);

// 		if (maxTMin < t1) maxTMin = t1;
// 		if (minTMax > t2) minTMax = t2;
// 		if (maxTMax < t2) maxTMax = t2;
// 	}
// 	if (dz > MYEPSILON)
// 	{
// 		float t1 = A_RS.z / ray->direction.z;
// 		float t2 = B_RS.z / ray->direction.z;
// 		if (t1 > t2)
// 			MySwap(t1, t2);

// 		if (maxTMin < t1) maxTMin = t1;
// 		if (minTMax > t2) minTMax = t2;
// 		if (maxTMax < t2) maxTMax = t2;
// 	}

// 	if (maxTMin < minTMax + MYEPSILON && maxTMax > MYEPSILON)
// 		return true;
// 	else
// 		return false;
// }

// // this function is inefficient and not precise
// static void BestCandidateAlgorithm(std::vector<glm::vec2> &point, int num, float w, float h)
// {
// 	// candidate num is 2 here
// 	int candidateNum = 2;

// 	point.clear();
// 	if (num == 0)
// 		return;

// 	srand(time(0));
// 	point.push_back(glm::vec2(rand() * w / RAND_MAX - w / 2, rand() * h / RAND_MAX - h / 2));
// 	for (int i = 1; i < num; i++)
// 	{
// 		float maxDis = 0;
// 		glm::vec2 curCandidate;
// 		for (int j = 0; j < candidateNum; j++)
// 		{
// 			glm::vec2 tmpCandidate(rand() * w / RAND_MAX - w / 2, rand() * h / RAND_MAX - h / 2);
// 			for (std::vector<glm::vec2>::iterator k = point.begin(); k != point.end(); k++)
// 			{
// 				float tmpDis = length(*k - tmpCandidate);
// 				if (maxDis < tmpDis)
// 				{
// 					maxDis = tmpDis;
// 					curCandidate = tmpCandidate;
// 				}
// 			}
// 		}
// 		point.push_back(curCandidate);
// 	}
// }

}
