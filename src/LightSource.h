#pragma once

#include <vector>
#include <memory>
#include "Type.h"

namespace ray_tracing {

class Ray;
struct RayHitRecord;

struct LightInfoToPoint {
  LightInfoToPoint(const Vec3f &c, const Vec3f &dir, float d)
  : color(c)
  , direction(dir)
  , distance(d) {
  }

  Vec3f color;
  Vec3f direction;
  float distance;
};

// todo, ?we normalize light color in unit voxel space, may be too big?!

class LightBase {
public:
  virtual ~LightBase() = default;

  virtual void AppendToLightInfo(const Vec3f &s_point,
    std::vector<LightInfoToPoint> &infos) const = 0;

  // virtual RayHitRecord RayIntersection(const Ray &ray) = 0;
};

//// todo, ?a voxel point light, will a voxel be toooooo big?
class PointLight : public LightBase {
public:
  PointLight(const Vec3f &pos, const Vec3f &color);

  void AppendToLightInfo(const Vec3f &s_point,
    std::vector<LightInfoToPoint> &infos) const override;

  // RayHitRecord RayIntersection(const Ray &ray) override;

private:
  Vec3f pos_;
  Vec3f color_;
};

class AreaLight : public LightBase {
public:
  AreaLight(const Vec3f &pos, const Vec3f &sum_color,
    const Vec2f &size, const Vec3f &right_dir, const Vec3f &down_dir);

  void AppendToLightInfo(const Vec3f &s_point,
    std::vector<LightInfoToPoint> &infos) const override;

  // RayHitRecord RayIntersection(const Ray &ray) override;

private:
  void GenerateCandidate(const Vec2f &size,
    uint32_t num, std::vector<Vec2f> &points);

  std::vector<std::shared_ptr<PointLight>> samples_;
};

class CubeMapFace : public LightBase {
public:
  CubeMapFace(std::vector<std::vector<Vec3f>> &&data,
    const Vec3f &center, const Vec3f &right_dir,
    const Vec3f &down_dir, float size);

  void AppendToLightInfo(const Vec3f &s_point,
    std::vector<LightInfoToPoint> &infos) const override;

  // RayHitRecord RayIntersection(const Ray &ray) override;

private:
  std::vector<std::vector<Vec3f>> data_;
  Vec3f center_;
  Vec3f right_dir_;
  Vec3f down_dir_;
  float size_;
  Vec3f normal_;
  float D_;
  std::vector<std::shared_ptr<AreaLight>> light_samples_;
};

enum CubeMapFaceType {
  TOP,
  BOTTOM,
  LEFT,
  RIGHT,
  FORWARD,
  BACKWARD
};

class CubeMap : public LightBase {
public:
  CubeMap(const std::string &filepath, float size);

  void AppendToLightInfo(const Vec3f &s_point,
    std::vector<LightInfoToPoint> &infos) const override;

  // RayHitRecord RayIntersection(const Ray &ray) override;

private:
  std::shared_ptr<CubeMapFace> CreateFace(int face_type,
    int r_index, int c_index, bool row_inverse, bool col_inverse,
    float size, float *image_data);

  bool hdr_flag_;
  int width_, height_, dimension_, N_;
  // top, bottom, left, right, forward, backward
  std::vector<std::shared_ptr<CubeMapFace>> faces_;
};

}
