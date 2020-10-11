#include "GeometryObject.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <fmt/format.h>
#include "RayTracingCamera.h"
#include "Utils.h"

namespace {

const float EPSILON = 1e-4f; // todo, is there precision problem?

}

namespace ray_tracing {

GeometryObject::GeometryObject(const std::string &type_name,
  const Vec3f &color)
  : type_name_(type_name)
  , color_(color) {
}
const AABB &GeometryObject::GetBoundingBox() const {
  return aabb_;
}


Sphere::Sphere(const Vec3f &center, float radius, const Vec3f &color)
  : GeometryObject("sphere", color)
  , center_(center)
  , radius_(radius) {
  aabb_ = AABB(center_.array() - radius_, center_.array() + radius_);
}
RayHitRecord Sphere::RayIntersection(const Ray &ray) const {
  RayHitRecord record;

  Vec3f sc = ray.s_point - center_;
  const auto &d = ray.direction;

  // (sc + d * t - center) ^ 2 == radius ^ 2
  // At^2 + Bt + C = 0, solve t
  float A2 = d.squaredNorm() * 2.f;
  float B = 2.f * d.dot(sc);
  float C = sc.squaredNorm() - radius_ * radius_;

  float det = B * B - 2.f * A2 * C;
  if (det > 0.f) {
    float t = 0.f;
    float sqrt_det = sqrt(det);
    if (sqrt_det + B < 0.f) {
      t = (-B - sqrt_det) / A2;
    } else if (sqrt_det - B > 0.f) {
      t = (-B + sqrt_det) / A2;
    }

    if (t > 0.f) {
      record.depth = t;
      record.hit_point = ray.GetPoint(t);
      record.hit_normal = (record.hit_point - center_).normalized();
      record.r_direction = ray.direction -
        2.f * ray.direction.dot(record.hit_normal) * record.hit_normal;
      record.point_color = color_;
    }
  }

  return record;
}


Plane::Plane(const Vec4f &ABCD, const Vec3f &color)
  : GeometryObject("plane", color)
  , ABCD_(ABCD / ABCD.head<3>().norm()) {
  aabb_ = AABB(Vec3f::Constant(-MYINFINITE), Vec3f::Constant(MYINFINITE));
}
RayHitRecord Plane::RayIntersection(const Ray &ray) const {
  RayHitRecord record;

  const auto &sp = ray.s_point;
  const auto &d = ray.direction;

  float denominator = ABCD_.head<3>().dot(d);
  float numerator = -ABCD_(3) - ABCD_.head<3>().dot(sp);

  float t = numerator / denominator;
  if (t > EPSILON) {
    record.depth = t;
    record.hit_point = ray.GetPoint(t);
    record.hit_normal = ABCD_.head<3>();
    record.r_direction = ray.direction -
      2.f * ray.direction.dot(record.hit_normal) * record.hit_normal;
    record.point_color = color_;
  }

  return record;
}


Triangle::Triangle(const Vec3f &A, const Vec3f &B,
  const Vec3f &C, const Vec3f &color)
  : GeometryObject("triangle", color)
  , A_(A)
  , B_(B)
  , C_(C) {
  aabb_.extend(A_).extend(B_).extend(C_);

  bary_center_ = (A_ + B_ + C_) / 3.f;
  edge1_ = B_ - A_;
  edge2_ = C_ - A_;
  normal_ = edge1_.cross(edge2_).normalized();
}
// Möller–Trumbore intersection algorithm
RayHitRecord Triangle::RayIntersection(const Ray &ray) const {
  RayHitRecord record;

  const auto &ray_o = ray.s_point;
  const auto &ray_d = ray.direction;

  if (ray_d.dot(normal_) >= 0.f) {
    return record;
  }

  Vec3f h = ray_d.cross(edge2_);
  float a = edge1_.dot(h);
  if (abs(a) < EPSILON) {
    return record;
  }

  float f = 1.f / a;
  Vec3f s = ray_o - A_;
  float u = f * s.dot(h);
  if (u < 0.f || u > 1.f) {
    return record;
  }

  Vec3f q = s.cross(edge1_);
  float v = f * ray_d.dot(q);
  if (v < 0.f || u + v > 1.f) {
    return record;
  }

  float t = f * edge2_.dot(q);
  if (t > EPSILON) {
    record.depth = t;
    record.hit_point = ray.GetPoint(t);
    record.hit_normal = normal_;
    record.r_direction = ray.direction -
      2.f * ray.direction.dot(record.hit_normal) * record.hit_normal;
    record.point_color = color_;
  }

  return record;
}
const Vec3f &Triangle::GetBaryCenter() const {
  return bary_center_;
}


Mesh::Mesh(const std::vector<Vec3f> &vertices,
  const std::vector<uint32_t> &faces_ids, const Vec3f &color)
  : GeometryObject("mesh", color) {
  for (auto i = 0; i < faces_ids.size(); i += 3u) {
    triangles_.emplace_back(vertices[faces_ids[i]],
      vertices[faces_ids[i + 1]], vertices[faces_ids[i + 2]], color);
  }

  tree_ = std::make_shared<Kdtree>(triangles_);
}
RayHitRecord Mesh::RayIntersection(const Ray &ray) const {
  RayHitRecord record;

  std::stack<std::shared_ptr<TreeNode>> nodes;
  nodes.emplace(tree_->GetRootNode());
  while (!nodes.empty()) {
    auto node = nodes.top();
    nodes.pop();

    if (node->RayHitAABB(ray)) {
      if (node->IsLeafNode()) {
        const auto &triangle_ids = node->GetTriangleIds();
        for (auto id : triangle_ids) {
          RayHitRecord tmp = triangles_[id].RayIntersection(ray);
          if (tmp.depth > EPSILON &&
            (record.depth > tmp.depth || record.depth < 0.f)) {
            record = std::move(tmp);
          }
        }
      } else {
        node->EmplaceNonEmptyChilds(nodes);
      }
    }
  }

  return record;
}


Model::Model(const std::string &filepath, const Vec3f &color)
  : GeometryObject("model", color) {
  srand(time(0)); // todo, necessary?

  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(filepath,
    aiProcess_Triangulate |
    aiProcess_JoinIdenticalVertices |
    aiProcess_SplitLargeMeshes |
    aiProcess_OptimizeMeshes);

  if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE ||
    !scene->mRootNode) {
    throw std::runtime_error(
      fmt::format("ERROR::ASSIMP::{}", importer.GetErrorString()));
  }

  ProcessNode(scene->mRootNode, scene);
}
RayHitRecord Model::RayIntersection(const Ray &Ray) const {
  RayHitRecord record;
  for (auto i = 0; i < meshes_.size(); ++i) {
    RayHitRecord tmp = meshes_[i]->RayIntersection(Ray);
    if (tmp.depth > EPSILON &&
      (record.depth > tmp.depth || record.depth < 0.f)) {
      record = std::move(tmp);
    }
  }
  return record;
}
void Model::ProcessNode(aiNode *root_node, const aiScene *scene) {
  std::stack<aiNode*> nodes;
  nodes.emplace(root_node);
  while (!nodes.empty()) {
    auto node = nodes.top();
    nodes.pop();
    // Process all the node's meshes (if any)
    for (auto i = 0; i < node->mNumMeshes; ++i) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      meshes_.emplace_back(CreateMesh(mesh));
    }
    for (auto i = 0; i < node->mNumChildren; ++i) {
      nodes.emplace(node->mChildren[i]);
    }
  }
}
std::shared_ptr<Mesh> Model::CreateMesh(aiMesh *mesh) {
  std::vector<Vec3f> vertices(mesh->mNumVertices);
  for (auto i = 0; i < mesh->mNumVertices; ++i) {
    vertices[i] << mesh->mVertices[i].x,
      mesh->mVertices[i].y,
      mesh->mVertices[i].z;
  }

  std::vector<uint32_t> faces_ids(3u * mesh->mNumFaces);
  for (auto i = 0, i1 = 0; i < mesh->mNumFaces; ++i) {
    const aiFace &face = mesh->mFaces[i];
    assert(face.nNumIndices == 3u);
    // Retrieve all indices of the face and store them in the indices vector
    for (auto j = 0; j < face.mNumIndices; ++j) {
      faces_ids[i1++] = face.mIndices[j];
    }
  }

  return std::make_shared<Mesh>(vertices, faces_ids,
    (Vec3f::Random() + Vec3f::Ones()) / 2.f);
}

}
