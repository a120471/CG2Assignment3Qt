// Define geometry types
#pragma once

#include <string>
#include "Kdtree.h"

class aiScene;
class aiMesh;
class aiNode;

namespace ray_tracing {

class Ray;

// Ray hit info
struct RayHitRecord {
  RayHitRecord& operator=(const RayHitRecord& record) {
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
  Vec3f hit_point{Vec3f::Zero()};
  Vec3f hit_normal{Vec3f::Zero()};
  Vec3f r_direction{Vec3f::Zero()};
  Vec3f point_color{Vec3f::Zero()};
};


// Base class
class GeometryObject {
public:
  GeometryObject(const std::string &type_name, const Vec3f &color);
  virtual ~GeometryObject() = default;

  virtual RayHitRecord RayIntersection(const Ray &ray) const = 0;

  // Use bounding box to accerlerate the ray hit test
  virtual const AABB &GetBoundingBox() const;

  std::string type_name_;
  Vec3f color_;
  AABB aabb_;
};

class Sphere : public GeometryObject {
public:
  Sphere(const Vec3f &center, float radius, const Vec3f &color);

  RayHitRecord RayIntersection(const Ray &ray) const override;

private:
  Vec3f center_;
  float radius_;
};

class Plane : public GeometryObject {
public:
  Plane(const Vec4f &ABCD, const Vec3f &color);

  RayHitRecord RayIntersection(const Ray &ray) const override;

private:
  // Ax + By + Cz + D = 0; ABC's norm should be 1.f
  Vec4f ABCD_;
};

class Triangle : public GeometryObject {
public:
  Triangle(const Vec3f &A, const Vec3f &B,
    const Vec3f &C, const Vec3f &color);

  RayHitRecord RayIntersection(const Ray &ray) const override;

  const Vec3f &GetBaryCenter() const;

private:
  Vec3f A_, B_, C_;
  Vec3f edge1_, edge2_;
  Vec3f normal_;
  Vec3f bary_center_;
};

class Mesh : public GeometryObject {
public:
  Mesh(const std::vector<Vec3f> &vertices,
    const std::vector<uint32_t> &indices,
    const Vec3f &color);

  RayHitRecord RayIntersection(const Ray &Ray) const override;

private:
  std::shared_ptr<Kdtree> tree_;
  std::vector<Triangle> triangles_;
};

class Model : public GeometryObject {
public:
  Model(const std::string &filepath, const Vec3f &color);

  RayHitRecord RayIntersection(const Ray &Ray) const override;

private:
  void ProcessNode(aiNode *node, const aiScene *scene);
  std::shared_ptr<Mesh> CreateMesh(aiMesh *mesh);

  std::vector<std::shared_ptr<Mesh>> meshes_;
};

}
