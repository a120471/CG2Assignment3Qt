#pragma once

#include "Type.h"

namespace ray_tracing {

struct Ray {
  Ray() = default;
  Ray(const Vec3f &origin_point, const Vec3f &dir);

  // Return a point on this ray
  inline Vec3f GetPoint(float t) const {
    return s_point + direction * t;
  }

  Vec3f s_point;
  Vec3f direction;
};

class RayTracingCamera {
public:
  RayTracingCamera(const Vec3f &pos, const Vec3f &lookat,
    const Vec3f &up = Vec3f(0.f, 1.f, 0.f));

  void SetK(const Vec2u &resolution, float fov_h);
  const Vec2u &GetResolution() const;

  // Generate the ray emit from pixel
  Ray GenerateRay(int row, int col) const;

private:
  Mat4f camera_frame_;
  Mat3f K_;
  Vec2u resolution_; // Pixel num
};

}
