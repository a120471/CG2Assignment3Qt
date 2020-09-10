// Define geometry types
#pragma once

#include <string>
#include <assimp/scene.h>
#include "KDTree.h"

namespace ray_tracing {

class Ray;

// saved data of each hit point and reflection direction
struct RayHitObjectRecord {
  RayHitObjectRecord(float depth = -1) {
    this->depth = depth;
  }

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

  float depth;
  Vec3f hit_point;
  Vec3f hit_normal;
  Vec3f r_direction;
  Vec3f point_color;
};


// Base class
class GeometryObject {
public:
  GeometryObject(const std::string &typeName, const Vec3f &color);
  virtual ~GeometryObject() = default;

  virtual void RayIntersection(const Ray &ray, RayHitObjectRecord &record) = 0;

  // we need to save the bounding box to accerlerate the ray hit test
  virtual void GetBoundingBox(Vec3f &AA, Vec3f &BB) = 0;

  // void getMaterial();

  std::string typeName_;
  Vec3f color_;
  Vec3f AA_, BB_; // bounding box
  //Mat4f transformMatrix;
};

class Sphere : public GeometryObject {
public:
  Sphere(const Vec3f &center, float radius,
    const Vec3f &color = Vec3f(1, 1, 1));

  void RayIntersection(const Ray &ray, RayHitObjectRecord &record) override;

  void GetBoundingBox(Vec3f &AA, Vec3f &BB) override;

private:
  Vec3f center_;
  float radius_;
};

class Plane : public GeometryObject {
public:
  Plane(const Vec4f &ABCD, const Vec3f &color = Vec3f(1, 1, 1));

  void RayIntersection(const Ray &ray, RayHitObjectRecord &record) override;

  void GetBoundingBox(Vec3f &AA, Vec3f &BB) override;

  Vec3f normal_;

private:
  // Ax + By + Cz + D = 0;
  Vec4f ABCD_;
};

class Triangle : public GeometryObject {
public:
  struct Vertex {
    Vec3f Position;
    Vec3f Normal;
  };

  Triangle(const Vertex &A, const Vertex &B, const Vertex &C,
    Vec3f color = Vec3f(1, 1, 1));

  void RayIntersection(const Ray &ray, RayHitObjectRecord &record) override;

  void GetBoundingBox(Vec3f &AA, Vec3f &BB) override;

  Vec3f baryCenter_;

private:
  Vertex A_, B_, C_;
  Vec3f eAB_, eAC_;
};

class Mesh : public GeometryObject {
public:
  Mesh(const std::vector<Triangle::Vertex> &vertices,
    const std::vector<int> &indices,
    const Vec3f &color = Vec3f(1, 1, 1));
  virtual ~Mesh();

  void RayIntersection(const Ray &Ray, RayHitObjectRecord &record) override;

  void HitTree(const Ray &ray, KDTree::TreeNode *node, RayHitObjectRecord &record);

  void GetBoundingBox(Vec3f &AA, Vec3f &BB) override { /*to do*/ }

  inline static bool SortByX(const Triangle *t1, const Triangle *t2) {
    return t1->baryCenter_[0] < t2->baryCenter_[0];
  }
  inline static bool SortByY(const Triangle *t1, const Triangle *t2) {
    return t1->baryCenter_[1] < t2->baryCenter_[1];
  }
  inline static bool SortByZ(const Triangle *t1, const Triangle *t2) {
    return t1->baryCenter_[2] < t2->baryCenter_[2];
  }

private:
  std::vector<Triangle*> faceTriangles_;
  KDTree *sKDT_;
};

class Model : public GeometryObject {
public:
  Model(const std::string &modelPath,
    const Vec3f &color = Vec3f(1, 1, 1));
  virtual ~Model();

  void RayIntersection(const Ray &Ray, RayHitObjectRecord &record) override;

  void GetBoundingBox(Vec3f &AA, Vec3f &BB) override { /*to do*/ }

private:
  void processNode(aiNode* node, const aiScene* scene);
  Mesh* processMesh(aiMesh* mesh, const aiScene* scene);

  std::vector<Mesh*> meshes_;
};

}
