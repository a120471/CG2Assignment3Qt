#include "GeometryObject.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Eigen/Geometry>
#include "Utils.h"

namespace ray_tracing {

GeometryObject::GeometryObject(const std::string &type_name,
  const Vec3f &color)
  : type_name_(type_name)
  , color_(color) {
}
void GeometryObject::GetBoundingBox(Vec3f &AA, Vec3f &BB) {
  AA = AA_;
  BB = BB_;
}


Sphere::Sphere(const Vec3f &center, float radius, const Vec3f &color)
  : GeometryObject("sphere", color)
  , center_(center)
  , radius_(radius) {
  AA_ = center_ - Vec3f::Ones() * radius_;
  BB_ = center_ + Vec3f::Ones() * radius_;
}
void Sphere::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
  Vec3f sc = ray.s_point - center_;
  Vec3f d = ray.direction;

  // (s_point + d * t - center) ^ 2 == radius ^ 2
  // At^2 + Bt + C = 0, solve t
  float A = d.squaredNorm();
  float B = 2 * d.dot(sc);
  float C = sc.squaredNorm() - radius_*radius_;

  float det = B*B - 4 * A*C;
  if (det > MYEPSILON) {
    float t1 = (-B - sqrt(det)) / (2 * A);
    float t2 = (-B + sqrt(det)) / (2 * A);

    if (t1 > MYEPSILON) {
      rhor.hit_point = ray.GetPoint(t1);
      rhor.hit_normal = (rhor.hit_point - center_).normalized();
      rhor.r_direction = ray.direction - 2 * ray.direction.dot(rhor.hit_normal) * rhor.hit_normal; // it's already normalized
      rhor.point_color = color_;
      rhor.depth = t1;
      return;
    }
    else if (t2 > MYEPSILON) {
      rhor.hit_point = ray.GetPoint(t2);
      rhor.hit_normal = (rhor.hit_point - center_).normalized();
      rhor.r_direction = ray.direction - 2 * ray.direction.dot(rhor.hit_normal) * rhor.hit_normal; // it's already normalized
      rhor.point_color = color_;
      rhor.depth = t2;
      return;
    }
  }

  rhor.hit_point = Vec3f::Zero();
  rhor.hit_normal = Vec3f::Zero();
  rhor.r_direction = Vec3f::Zero();
  rhor.point_color = Vec3f::Zero();
  rhor.depth = -1.f;
}


Plane::Plane(const Vec4f &ABCD, const Vec3f &color)
  : GeometryObject("plane", color)
  , ABCD_(ABCD) {
  AA_ = Vec3f::Ones() * -MYINFINITE;
  BB_ = Vec3f::Ones() * MYINFINITE;
}
void Plane::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
  Vec3f sp = ray.s_point;
  Vec3f d = ray.direction;
  auto normal = ABCD_.head<3>().normalized();

  float denominator = ABCD_.head<3>().dot(d);
  float numerator = -ABCD_[3] - ABCD_.head<3>().dot(sp);

  float t = numerator / denominator;
  if (t > MYEPSILON) {
    rhor.hit_point = ray.GetPoint(t);
    rhor.hit_normal = normal;
    rhor.r_direction = ray.direction - 2 * ray.direction.dot(rhor.hit_normal) * rhor.hit_normal; // it's already normalized
    rhor.point_color = color_;
    rhor.depth = t;
    return;
  }

  rhor.hit_point = Vec3f::Zero();
  rhor.hit_normal = Vec3f::Zero();
  rhor.r_direction = Vec3f::Zero();
  rhor.point_color = Vec3f::Zero();
  rhor.depth = -1.f;
}


Triangle::Triangle(const Vertex &A,
  const Vertex &B, const Vertex &C, Vec3f color)
  : GeometryObject("triangle", color)
  , A_(A)
  , B_(B)
  , C_(C) {
  AA_ = A_.position.cwiseMin(B_.position).cwiseMin(C_.position);
  BB_ = A_.position.cwiseMax(B_.position).cwiseMax(C_.position);

  bary_center_ = (A_.position + B_.position + C_.position) / 3.0f;
  eAB_ = B_.position - A_.position;
  eAC_ = C_.position - A_.position;
}
void Triangle::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
  Vec3f s = ray.s_point - A_.position;
  Vec3f d = ray.direction;

  float denominator = d.cross(eAC_).dot(eAB_);

  float b1 = d.cross(eAC_).dot(s) / denominator;
  float b2 = eAB_.cross(d).dot(s) / denominator;
  float t = eAB_.cross(eAC_).dot(s) / denominator;
  if (t > MYEPSILON && b1 > -MYEPSILON && b2 > -MYEPSILON && b1 + b2 < 1 + MYEPSILON) {
    rhor.hit_point = ray.GetPoint(t);
    rhor.hit_normal = ((1 - b1 - b2) * A_.normal + b1 * B_.normal + b2 * C_.normal).normalized();
    //rhor.hit_normal = eAB_.cross(eAC_).normalized();
    rhor.r_direction = ray.direction - 2 * ray.direction.dot(rhor.hit_normal) * rhor.hit_normal; // it's already normalized
    rhor.point_color = color_;
    rhor.depth = t;
    return;
  }

  rhor.hit_point = Vec3f::Zero();
  rhor.hit_normal = Vec3f::Zero();
  rhor.r_direction = Vec3f::Zero();
  rhor.point_color = Vec3f::Zero();
  rhor.depth = -1.f;
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
void Mesh::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
  KDTree::TreeNode *node = tree_->GetRootNode();
  HitTree(ray, node, rhor);
}
void Mesh::HitTree(const Ray &ray, KDTree::TreeNode *node,
  RayHitObjectRecord &rhor) {
  if (RayHitAABB(ray, node->AA, node->BB)) {
    if (!node->face_ids.empty()) {
      RayHitObjectRecord rhorT;
      for (auto i = node->face_ids.begin(); i != node->face_ids.end(); ++i) {
        triangles_[*i]->RayIntersection(ray, rhorT);
        if (rhorT.depth > MYEPSILON &&
          (rhor.depth > rhorT.depth || rhor.depth < MYEPSILON)) {
          rhor = rhorT;
        }
      }
    } else {
      HitTree(ray, node->l_child, rhor);
      HitTree(ray, node->r_child, rhor);
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
void Model::RayIntersection(const Ray &Ray, RayHitObjectRecord &rhor) {
  RayHitObjectRecord rhorT;
  for (auto i = 0; i < meshes_.size(); ++i) {
    meshes_[i]->RayIntersection(Ray, rhorT);
    if (rhorT.depth > MYEPSILON && (rhor.depth > rhorT.depth || rhor.depth < MYEPSILON)) {
      rhor = rhorT;
    }
  }
}
void Model::ProcessNode(aiNode *node, const aiScene *scene) {
  // Process all the node's meshes (if any)
  for (auto i = 0; i < node->mNumMeshes; ++i) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes_.emplace_back(CreateMesh(mesh, scene));
  }
  // Then do the same for each of its children
  for (auto i = 0; i < node->mNumChildren; ++i) {
    ProcessNode(node->mChildren[i], scene);
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
