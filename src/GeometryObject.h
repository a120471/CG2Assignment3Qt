// Define geometry types
#pragma once

#include <string>
#include <assimp/scene.h>
#include "KDTree.h"

namespace ray_tracing {

class Ray;

// saved data of each hit point and reflection direction
struct RayHitObjectRecord {
  RayHitObjectRecord& operator=(const RayHitObjectRecord& record) {
    if (this == &record) {
      return *this;
    }
    this->depth = record.depth;
    this->hit_point = record.hit_point;
    this->hit_normal = record.hit_normal;
    this->r_direction = record.r_direction;
    this->point_color = record.point_color;

    return *this;
  }

  float depth{-1.f};
  Vec3f hit_point;
  Vec3f hit_normal;
  Vec3f r_direction;
  Vec3f point_color;
};


// Base class
class GeometryObject {
public:
  GeometryObject(const std::string &type_name, const Vec3f &color);
  virtual ~GeometryObject() = default;

  virtual void RayIntersection(const Ray &ray, RayHitObjectRecord &record) = 0;

  // use bounding box to accerlerate the ray hit test
  virtual void GetBoundingBox(Vec3f &AA, Vec3f &BB);

  std::string type_name_;
  Vec3f color_;
  Vec3f AA_, BB_; // bounding box
};

class Sphere : public GeometryObject {
public:
  Sphere(const Vec3f &center, float radius,
    const Vec3f &color = Vec3f::Ones());

  void RayIntersection(const Ray &ray, RayHitObjectRecord &record) override;

private:
  Vec3f center_;
  float radius_;
};

class Plane : public GeometryObject {
public:
  Plane(const Vec4f &ABCD, const Vec3f &color = Vec3f::Ones());

  void RayIntersection(const Ray &ray, RayHitObjectRecord &record) override;

private:
  // Ax + By + Cz + D = 0;
  Vec4f ABCD_;
};

class Triangle : public GeometryObject {
public:
  struct Vertex {
    Vec3f position;
    Vec3f normal;
  };

  Triangle(const Vertex &A, const Vertex &B, const Vertex &C,
    Vec3f color = Vec3f::Ones());

  void RayIntersection(const Ray &ray, RayHitObjectRecord &record) override;

  const Vec3f &GetBaryCenter() const;

private:
  Vertex A_, B_, C_;
  Vec3f eAB_, eAC_;
  Vec3f bary_center_;
};

class Mesh : public GeometryObject {
public:
  Mesh(const std::vector<Triangle::Vertex> &vertices,
    const std::vector<int> &indices,
    const Vec3f &color = Vec3f::Ones());

  void RayIntersection(const Ray &Ray, RayHitObjectRecord &record) override;

  void HitTree(const Ray &ray, KDTree::TreeNode *node, RayHitObjectRecord &record);

private:
  std::vector<std::shared_ptr<Triangle>> triangles_;
  std::shared_ptr<KDTree> tree_;
};

class Model : public GeometryObject {
public:
  Model(const std::string &filepath,
    const Vec3f &color = Vec3f::Ones());

  void RayIntersection(const Ray &Ray, RayHitObjectRecord &record) override;

private:
  void ProcessNode(aiNode *node, const aiScene *scene);
  std::shared_ptr<Mesh> CreateMesh(aiMesh *mesh, const aiScene *scene);

  std::vector<std::shared_ptr<Mesh>> meshes_;
};

}
