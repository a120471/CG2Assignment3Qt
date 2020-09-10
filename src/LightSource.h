#pragma once

#include <vector>
#include "Type.h"

namespace ray_tracing {

class Ray;
struct RayHitObjectRecord;

// we normalize light color in unit voxel space, may be too big?!
class LightBase {
public:
  virtual ~LightBase() = default;

  virtual void GetLight(const Vec3f &s_point,
    std::vector<Vec3f> &colors,
    std::vector<float> &distances,
    std::vector<Vec3f> &light_dirs) = 0;

  virtual void RayIntersection(const Ray &ray, RayHitObjectRecord &record) = 0;
};

// a voxel point light, will a voxel be toooooo big?
class PointLight : public LightBase {
public:
  PointLight(const Vec3f &pos, const Vec3f &color);

  void GetLight(const Vec3f &s_point,
    std::vector<Vec3f> &colors,
    std::vector<float> &distances,
    std::vector<Vec3f> &light_dirs) override;

  void RayIntersection(const Ray &ray, RayHitObjectRecord &record) override;

private:
  Vec3f pos_;
  Vec3f color_;
};

// class AreaLight : public LightBase {
// public:
//   AreaLight(Vec2f areaWH, Vec2i resoWH, Vec3f pos, Vec3f totalColor, Vec3f dRight, Vec3f dDown);
//   virtual ~AreaLight();

//   void GetLight(Vec3f, std::vector<Vec3f>&, std::vector<float>&, std::vector<Vec3f>&) override;

//   void RayIntersection(const Ray &ray, RayHitObjectRecord&) override;

// private:
//   Vec3f unitColor, pos, normal;
//   float w, h;
//   std::vector<PointLight*> pointSamples;
// };

// class SquareMap : public LightBase {
// public:
//   SquareMap(Vec3f **data, int n, Vec3f ulCorner, Vec3f dRight, Vec3f dDown, float size);
//   virtual ~SquareMap();

//   void GetLight(Vec3f, std::vector<Vec3f>&, std::vector<float>&, std::vector<Vec3f>&) override;

//   void RayIntersection(const Ray &ray, RayHitObjectRecord&) override;

// private:
//   Vec3f **data;
//   int n;
//   Vec3f ulCorner, dRight, dDown;
//   float size;
//   Vec3f normal;
//   float D;
//   QuadTree* quadT;
//   std::vector<AreaLight*> lightSamples;
// };

// class CubeMap : public LightBase {
// public:
//   CubeMap(std::string cubeMapPath, float size);
//   CubeMap(std::string cubeMapPath[]);
//   virtual ~CubeMap();

//   void GetLight(Vec3f, std::vector<Vec3f>&, std::vector<float>&, std::vector<Vec3f>&) override;

//   void RayIntersection(const Ray &ray, RayHitObjectRecord&) override;

// private:
//   float *loadImage;
//   int width, height, dimension, N;
//   SquareMap *top;
//   SquareMap *bottom;
//   SquareMap *left;
//   SquareMap *right;
//   SquareMap *forward;
//   SquareMap *backward;

//   void ExtractSquareMap(SquareMap *&sm, int smIdx, int rowIdx, int colIdx, bool rowInverse, bool colInverse, float size);
// };

}
