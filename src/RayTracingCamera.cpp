#include "RayTracingCamera.h"
#include <Eigen/Geometry>

namespace ray_tracing {

Ray::Ray(const Vec3f &origin_point, const Vec3f &dir)
  : s_point(origin_point)
  , direction(dir.normalized()) {
}


RayTracingCamera::RayTracingCamera(
  const Vec3f &pos,
  const Vec3f &lookat,
  const Vec3f &up,
  int multi_samling_level_)
  : pos_(pos) {
  front_ = (lookat - pos).normalized();
  up_ = up.normalized();

  ir_ = front_.cross(up_).normalized();
  id_ = front_.cross(ir_).normalized();
  has_pixel_size_ = false;

  multi_samling_level_ = (int)sqrt(multi_samling_level_);
}

// row and col start from 0
void RayTracingCamera::GenerateRay(int row, int col, std::vector<Ray> &rays) {
  if (!has_pixel_size_) {
    has_pixel_size_ = true;
    pixel_width_ = whf_(0) / resolution_(0);
    pixel_height_ = whf_(1) / resolution_(1);
  }

  Vec3f colOffset = Vec3f(ir_ * (col - (resolution_(0) - 1.0f) / 2)) * pixel_width_;
  Vec3f rowOffset = Vec3f(id_ * (row - (resolution_(1) - 1.0f) / 2)) * pixel_height_;
  Vec3f ePoint = p_ + colOffset + rowOffset;

  rays.clear();
  float step = 1.0f / (multi_samling_level_ + 1);
  for (int i = 1; i <= multi_samling_level_; ++i) {
    for (int j = 1; j <= multi_samling_level_; ++j) {
      Vec3f colOffsetLocal = Vec3f(ir_ * (i * step - 0.5f)) * pixel_width_;
      Vec3f rowOffsetLocal = Vec3f(id_ * (j * step - 0.5f)) * pixel_height_;

      Vec3f directionLocal = (ePoint + colOffsetLocal + rowOffsetLocal - pos_).normalized();
      rays.emplace_back(pos_, directionLocal);
    }
  }
}

const Vec2u &RayTracingCamera::GetResolution() {
  return resolution_;
}

void RayTracingCamera::SetResolution(const Vec2u &resolution) {
  resolution_ = resolution;
  has_pixel_size_ = false;
}

const Vec3f &RayTracingCamera::GetWHF() {
  return whf_;
}

void RayTracingCamera::SetWHF(const Vec3f &whf) {
  whf_ = whf;
  has_pixel_size_ = false;
}

int RayTracingCamera::GetRayNumEachPixel() {
  return multi_samling_level_ * multi_samling_level_;
}

}
