#pragma once

#include <vector>
#include "Type.h"

namespace ray_tracing {

class Ray {
public:
  Ray(const Vec3f &origin_point, const Vec3f &dir);
  ~Ray() = default;

  // return a point on this ray
  inline Vec3f GetPoint(float t) const {
    return s_point + direction * t;
  }

  Vec3f s_point;
  Vec3f direction;
};

class RayTracingCamera {
public:
  RayTracingCamera(const Vec3f &pos = Vec3f::Zero(),
    const Vec3f &lookat = Vec3f(0.f, 0.f, -1.f),
    const Vec3f &up = Vec3f(0.f, 1.f, 0.f),
    int multi_sampling = 1);
  ~RayTracingCamera() = default;

  Vec3f getPos() {return pos_;}
  Vec3f getFront() {return front_;}
  Vec3f getUp() {return up_;}
  void SetCameraPos(Vec3f pos, Vec3f lookat, Vec3f up = Vec3f(0, 1, 0))  {
    pos_ = pos;
    front_ = (lookat - pos_).normalized();
    up_ = (up_).normalized();

    ir_ = front_.cross(up_).normalized();
    id_ = front_.cross(ir_).normalized();
  }

  const Vec2u &GetResolution();
  void SetResolution(const Vec2u &resolution);
  const Vec3f &GetWHF();
  void SetWHF(const Vec3f &whf);
  Vec3f GetP() {return p_;}
  void SetP(Vec3f p) {p_ = p;}
  int GetRayNumEachPixel();

  // compute the list of rays emit from pixel (i, j)
  void GenerateRay(int row, int col, std::vector<Ray> &rays);

private:
  // camera location and orientation
  Vec3f pos_, front_, up_;
  // number of pixels
  Vec2u resolution_;
  // image width, image height, local length
  Vec3f whf_;
  // image center position
  Vec3f p_;

  // image right direction, image down direction
  Vec3f ir_, id_;
  bool has_pixel_size_ = false;
  float pixel_width_, pixel_height_;

  // decide how many rays are generated in each pixel
  int multi_samling_level_;
};

}
