#include "RayTracingCamera.h"

namespace ray_tracing {

Ray::Ray(const Vec3f &origin_point, const Vec3f &dir)
  : s_point(origin_point)
  , direction(dir.normalized()) {
}


RayTracingCamera::RayTracingCamera(
  const Vec3f &pos,
  const Vec3f &lookat,
  const Vec3f &up) {

  auto z_dir = (pos - lookat).normalized();
  auto x_dir = up.cross(z_dir).normalized();
  auto y_dir = z_dir.cross(x_dir).normalized();

  camera_frame_ << x_dir(0), y_dir(0), z_dir(0), pos(0),
                   x_dir(1), y_dir(1), z_dir(1), pos(1),
                   x_dir(2), y_dir(2), z_dir(2), pos(2),
                   0.f, 0.f, 0.f, 1.f;
}

void RayTracingCamera::SetK(const Vec2u &resolution, float fov_h) {
  resolution_ = resolution;

  float focal = (resolution_(0) / 2.f) / tan(fov_h / 2.f);
  K_ << focal, 0.f, resolution_(0) / 2.f - 0.5f,
        0.f, focal, resolution_(1) / 2.f - 0.5f,
        0.f, 0.f, 1.f;
}

const Vec2u &RayTracingCamera::GetResolution() {
  return resolution_;
}

// row and col start from 0
void RayTracingCamera::GenerateRay(int row, int col, Ray &ray) {

  Vec3f local_ray = K_.inverse() * Vec3f(col, resolution_(1) - 1 - row, 1.f);
  local_ray(2) *= -1.f;

  ray.s_point = camera_frame_.col(3).head(3);
  ray.direction = (camera_frame_.block<3, 3>(0, 0) * local_ray).normalized();
}

}
