#include "GeometryObject.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "Utils.h"

namespace {

using namespace ray_tracing;

void SetInvalidReyHitObjectRecord(RayHitObjectRecord &record) {
  record.hit_point = Vec3f::Zero();
  record.hit_normal = Vec3f::Zero();
  record.r_direction = Vec3f::Zero();
  record.point_color = Vec3f::Zero();
  record.depth = -1.f;
}

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
  aabb_ = AABB(center_ - Vec3f::Constant(radius_),
    center_ + Vec3f::Constant(radius_));
}
void Sphere::RayIntersection(const Ray &ray, RayHitObjectRecord &record) {
  Vec3f sc = ray.s_point - center_;
  Vec3f d = ray.direction;

  // (s_point + d * t - center) ^ 2 == radius ^ 2
  // At^2 + Bt + C = 0, solve t
  float A = d.squaredNorm();
  float B = 2 * d.dot(sc);
  float C = sc.squaredNorm() - radius_*radius_;

  float det = B * B - 4 * A * C;
  if (det > MYEPSILON) {
    float t1 = (-B - sqrt(det)) / (2 * A);
    float t2 = (-B + sqrt(det)) / (2 * A);

    if (t1 > MYEPSILON) {
      record.hit_point = ray.GetPoint(t1);
      record.hit_normal = (record.hit_point - center_).normalized();
      record.r_direction = ray.direction - 2 * ray.direction.dot(record.hit_normal) * record.hit_normal; // it's already normalized
      record.point_color = color_;
      record.depth = t1;
      return;
    } else if (t2 > MYEPSILON) {
      record.hit_point = ray.GetPoint(t2);
      record.hit_normal = (record.hit_point - center_).normalized();
      record.r_direction = ray.direction - 2 * ray.direction.dot(record.hit_normal) * record.hit_normal; // it's already normalized
      record.point_color = color_;
      record.depth = t2;
      return;
    }
  }

  SetInvalidReyHitObjectRecord(record);
}


Plane::Plane(const Vec4f &ABCD, const Vec3f &color)
  : GeometryObject("plane", color)
  , ABCD_(ABCD) {
  aabb_ = AABB(Vec3f::Constant(-MYINFINITE),
    Vec3f::Constant(MYINFINITE));
}
void Plane::RayIntersection(const Ray &ray, RayHitObjectRecord &record) {
  Vec3f sp = ray.s_point;
  Vec3f d = ray.direction;

  float denominator = ABCD_.head<3>().dot(d);
  float numerator = -ABCD_(3) - ABCD_.head<3>().dot(sp);

  float t = numerator / denominator;
  if (t > MYEPSILON) {
    auto normal = ABCD_.head<3>().normalized();
    record.hit_point = ray.GetPoint(t);
    record.hit_normal = normal;
    record.r_direction = ray.direction - 2 * ray.direction.dot(record.hit_normal) * record.hit_normal; // it's already normalized
    record.point_color = color_;
    record.depth = t;
    return;
  }

  SetInvalidReyHitObjectRecord(record);
}


Triangle::Triangle(const Vertex &A,
  const Vertex &B, const Vertex &C, Vec3f color)
  : GeometryObject("triangle", color)
  , A_(A)
  , B_(B)
  , C_(C) {
  aabb_.extend(A_.position).extend(B_.position).extend(C_.position);

  bary_center_ = (A_.position + B_.position + C_.position) / 3.0f;
  edge1_ = B_.position - A_.position;
  edge2_ = C_.position - A_.position;
}
// Möller–Trumbore intersection algorithm
void Triangle::RayIntersection(const Ray &ray, RayHitObjectRecord &record) {
  const Vec3f &ray_o = ray.s_point;
  const Vec3f &ray_d = ray.direction;

  Vec3f h = ray_d.cross(edge2_);
  float a = edge1_.dot(h);
  if (abs(a) < MYEPSILON) {
    SetInvalidReyHitObjectRecord(record);
    return;
  }

  float f = 1.f / a;
  Vec3f s = ray_o - A_.position;
  float u = f * s.dot(h);
  if (u < 0.f || u > 1.f) {
    SetInvalidReyHitObjectRecord(record);
    return;
  }

  Vec3f q = s.cross(edge1_);
  float v = f * ray_d.dot(q);
  if (v < 0.f || u + v > 1.f) {
    SetInvalidReyHitObjectRecord(record);
    return;
  }

  float t = f * edge2_.dot(q);
  if (t > MYEPSILON) {
    record.hit_point = ray.GetPoint(t);
    record.hit_normal = ((1.f - u - v) * A_.normal +
      u * B_.normal + v * C_.normal).normalized();
    //record.hit_normal = edge1_.cross(edge2_).normalized();
    record.r_direction = ray.direction - 2 * ray.direction.dot(record.hit_normal) * record.hit_normal; // it's already normalized
    record.point_color = color_;
    record.depth = t;
    return;
  } else {
    SetInvalidReyHitObjectRecord(record);
    return;
  }
}
const Vec3f &Triangle::GetBaryCenter() const {
  return bary_center_;
}


Mesh::Mesh(const std::vector<Triangle::Vertex> &vertices,
  const std::vector<int> &faces, const Vec3f &color)
  : GeometryObject("mesh", color) {
  triangles_.clear();
  for (auto i = 0; i < faces.size(); i += 3) {
    triangles_.emplace_back(std::make_shared<Triangle>(vertices[faces[i]],
      vertices[faces[i + 1]], vertices[faces[i + 2]], color));
  }

  tree_ = std::make_shared<KDTree>(triangles_);
}
void Mesh::RayIntersection(const Ray &ray, RayHitObjectRecord &record) {
  KDTree::TreeNode *node = tree_->GetRootNode();
  HitTree(ray, node, record);
}
void Mesh::HitTree(const Ray &ray, KDTree::TreeNode *node,
  RayHitObjectRecord &record) {
  if (RayHitAABB(ray, node->aabb)) {
    if (!node->face_ids.empty()) {
      RayHitObjectRecord rhorT;
      for (auto id : node->face_ids) {
        triangles_[id]->RayIntersection(ray, rhorT);
        if (rhorT.depth > MYEPSILON &&
          (record.depth > rhorT.depth || record.depth < MYEPSILON)) {
          record = rhorT;
        }
      }
    } else {
      HitTree(ray, node->l_child, record);
      HitTree(ray, node->r_child, record);
    }
  }
}


Model::Model(const std::string &filepath, const Vec3f &color)
  : GeometryObject("model", color) {
  srand(time(0));

  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(filepath,
    aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
    aiProcess_FlipUVs | aiProcess_GenNormals |
    aiProcess_SplitLargeMeshes | aiProcess_OptimizeMeshes);

  if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE ||
    !scene->mRootNode) {
    //std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return;
  }

  meshes_.clear();
  ProcessNode(scene->mRootNode, scene);
}
void Model::RayIntersection(const Ray &Ray, RayHitObjectRecord &record) {
  RayHitObjectRecord rhorT;
  for (auto i = 0; i < meshes_.size(); ++i) {
    meshes_[i]->RayIntersection(Ray, rhorT);
    if (rhorT.depth > MYEPSILON && (record.depth > rhorT.depth || record.depth < MYEPSILON)) {
      record = rhorT;
    }
  }
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
      meshes_.emplace_back(CreateMesh(mesh, scene));
    }
    for (auto i = 0; i < node->mNumChildren; ++i) {
      nodes.emplace(node->mChildren[i]);
    }
  }
}
std::shared_ptr<Mesh> Model::CreateMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Triangle::Vertex> vertices;
  std::vector<int> faces;

  vertices.resize(mesh->mNumVertices);
  for (auto i = 0; i < mesh->mNumVertices; ++i) {
    Triangle::Vertex vertex;

    vertex.position << mesh->mVertices[i].x,
      mesh->mVertices[i].y,
      mesh->mVertices[i].z;

    vertex.normal << mesh->mNormals[i].x,
      mesh->mNormals[i].y,
      mesh->mNormals[i].z;

    vertices[i] = vertex;
  }
  // Process faces
  faces.resize(3 * mesh->mNumFaces);
  for (auto i = 0, i1 = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];
    // Retrieve all indices of the face and store them in the indices vector
    for (auto j = 0; j < face.mNumIndices; ++j) {
      faces[i1++] = face.mIndices[j];
    }
  }

  return std::make_shared<Mesh>(vertices, faces,
    Vec3f(static_cast<float>(rand()) / RAND_MAX,
      static_cast<float>(rand()) / RAND_MAX,
      static_cast<float>(rand()) / RAND_MAX));
}

}
