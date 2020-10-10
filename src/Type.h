#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace ray_tracing {

using Vec2u = Eigen::Matrix<uint32_t, 2, 1>;
using Vec2i = Eigen::Vector2i;
using Vec2f = Eigen::Vector2f;
using Vec3f = Eigen::Vector3f;
using Vec3d = Eigen::Vector3d;
using Vec4f = Eigen::Vector4f;
using Mat3f = Eigen::Matrix3f;
using Mat4f = Eigen::Matrix4f;
using AABB = Eigen::AlignedBox<float, 3>;

struct RenderParams {
  Vec2u resolution;
  uint32_t image_scale_ratio;
};

}
