#pragma once

#include <vector>
#include <ctime>
#include "RayTracingCamera.h"
#include "Type.h"

const float MYINFINITE = 999999.f;

const float MYEPSILON = 2e-4;

const float COLORINTENSITYTHRES = 2e4f;

// #ifndef UNITAREASAMPLECOLOR
// #define UNITAREASAMPLECOLOR (COLORINTENSITYTHRES / 10.0f)
// #endif // !UNITAREASAMPLECOLOR

const float NTHIDX = 0.85f;

const uint32_t MYTHREADNUM = 8u;

namespace ray_tracing {

extern bool hasHDRLighting;

template<typename T> void SafeDelete(T *&p) {
  delete p;
  p = nullptr;
}

inline bool SmallerThanEps(float a, float b) {
  return a < b - MYEPSILON;
}

inline bool BiggerThanEps(float a, float b) {
  return a > b + MYEPSILON;
}

inline bool ApproxiSmaller(float a, float b) {
  return a < b + MYEPSILON;
}

template <typename T>
void inline MySwap(T &t1, T &t2) {
  T tmp = t1;
  t1 = t2;
  t2 = tmp;
};

static void MergeBoundingBox(Vec3f &A, Vec3f &B,
  Vec3f A1, Vec3f B1, Vec3f A2, Vec3f B2) {
  A[0] = std::min(A1[0], A2[0]);
  A[1] = std::min(A1[1], A2[1]);
  A[2] = std::min(A1[2], A2[2]);

  B[0] = std::max(B1[0], B2[0]);
  B[1] = std::max(B1[1], B2[1]);
  B[2] = std::max(B1[2], B2[2]);
}

// Reference: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
static bool RayHitAABB(const Ray &ray, const Vec3f &A, const Vec3f &B) {
  float abs_dx = abs(ray.direction(0));
  float abs_dy = abs(ray.direction(1));
  float abs_dz = abs(ray.direction(2));

  Vec3f A_RS = A - ray.s_point;
  Vec3f B_RS = B - ray.s_point;

  if (abs_dx < MYEPSILON && (SmallerThanEps(ray.s_point(0), A(0)) || BiggerThanEps(ray.s_point(0), B(0)))) {
    return false;
  }
  if (abs_dy < MYEPSILON && (SmallerThanEps(ray.s_point(1), A(1)) || BiggerThanEps(ray.s_point(1), B(1)))) {
    return false;
  }
  if (abs_dz < MYEPSILON && (SmallerThanEps(ray.s_point(2), A(2)) || BiggerThanEps(ray.s_point(2), B(2)))) {
    return false;
  }
  float max_tmin = -MYINFINITE, min_tmax = MYINFINITE;

  bool valid = false;
  if (abs_dx > MYEPSILON) {
    float t1 = A_RS(0) / ray.direction(0);
    float t2 = B_RS(0) / ray.direction(0);
    if (t1 > t2) {
      MySwap(t1, t2);
    }
    max_tmin = std::max(max_tmin, t1);
    min_tmax = std::min(min_tmax, t2);
    if (t2 > 0) {
      valid = true;
    }
  }
  if (abs_dy > MYEPSILON) {
    float t1 = A_RS(1) / ray.direction(1);
    float t2 = B_RS(1) / ray.direction(1);
    if (t1 > t2) {
      MySwap(t1, t2);
    }
    max_tmin = std::max(max_tmin, t1);
    min_tmax = std::min(min_tmax, t2);
    if (t2 > 0) {
      valid = true;
    }
  }
  if (abs_dz > MYEPSILON) {
    float t1 = A_RS(2) / ray.direction(2);
    float t2 = B_RS(2) / ray.direction(2);
    if (t1 > t2) {
      MySwap(t1, t2);
    }
    max_tmin = std::max(max_tmin, t1);
    min_tmax = std::min(min_tmax, t2);
    if (t2 > 0) {
      valid = true;
    }
  }

  if (max_tmin < min_tmax + MYEPSILON && valid) {
    return true;
  } else {
    return false;
  }
}

// bool VoxelNode::CalculateRayHit(const Vec3f &ray_o, const Vec3f &normalized_d,
//   float &near_distance) const {
//   Vec3f min_corner = center_ - Vec3f::Ones() * side_len_;
//   Vec3f max_corner = center_ + Vec3f::Ones() * side_len_;
//   Vec3f min_dc = min_corner - ray_o;
//   Vec3f max_dc = max_corner - ray_o;
//   float max_tmin = -std::numeric_limits<float>::max();
//   float min_tmax = std::numeric_limits<float>::max();

//   for (int i = 0; i < 3; ++i) {
//     if (abs(normalized_d(i)) > EPSILON) {
//       float t1 = min_dc(i) / normalized_d(i);
//       float t2 = max_dc(i) / normalized_d(i);
//       if (t1 > t2) {
//         std::swap(t1, t2);
//       }
//       max_tmin = std::max(max_tmin, t1);
//       min_tmax = std::min(min_tmax, t2);
//     } else if (min_dc(i) * max_dc(i) > 0) {
//       return false;
//     }
//   }

//   if (ApproxiSmaller(max_tmin, min_tmax) && min_tmax > 0) {
//     near_distance = max_tmin;
//     return true;
//   } else {
//     return false;
//   }
// }

// this function is inefficient and not precise
static void BestCandidateAlgorithm(std::vector<Vec2f> &point,
  int num, float w, float h) {
  // candidate num is 2 here
  int candidateNum = 2;

  point.clear();
  if (num == 0) {
    return;
  }

  srand(time(0));
  point.emplace_back(rand() * w / RAND_MAX - w / 2, rand() * h / RAND_MAX - h / 2);
  for (int i = 1; i < num; ++i) {
    float maxDis = 0;
    Vec2f curCandidate;
    for (int j = 0; j < candidateNum; ++j) {
      Vec2f tmpCandidate(rand() * w / RAND_MAX - w / 2, rand() * h / RAND_MAX - h / 2);
      for (std::vector<Vec2f>::iterator k = point.begin(); k != point.end(); ++k) {
        float tmpDis = (*k - tmpCandidate).norm();
        if (maxDis < tmpDis) {
          maxDis = tmpDis;
          curCandidate = tmpCandidate;
        }
      }
    }
    point.emplace_back(curCandidate);
  }
}

}
