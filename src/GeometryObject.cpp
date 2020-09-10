#include "GeometryObject.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Eigen/Geometry>
#include "Utils.h"

namespace ray_tracing {

GeometryObject::GeometryObject(const std::string &typeName,
  const Vec3f &color)
  : typeName_(typeName)
  , color_(color) {
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

  rhor.hit_point = Vec3f(0, 0, 0);
  rhor.hit_normal = Vec3f(0, 0, 0);
  rhor.r_direction = Vec3f(0, 0, 0);
  rhor.point_color = Vec3f(0, 0, 0);
  rhor.depth = -1;
}
void Sphere::GetBoundingBox(Vec3f &AA, Vec3f &BB) {
  AA = AA;
  BB = BB;
}


Plane::Plane(const Vec4f &ABCD, const Vec3f &color)
  : GeometryObject("plane", color)
  , ABCD_(ABCD)
  , normal_(ABCD.head<3>().normalized()) {
  AA_ = Vec3f::Ones() * -MYINFINITE;
  BB_ = Vec3f::Ones() * MYINFINITE;
}
void Plane::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
  Vec3f sp = ray.s_point;
  Vec3f d = ray.direction;

  float denominator = ABCD_.head<3>().dot(d);
  float numerator = -ABCD_[3] - ABCD_.head<3>().dot(sp);

  float t = numerator / denominator;
  if (t > MYEPSILON) {
    rhor.hit_point = ray.GetPoint(t);
    rhor.hit_normal = normal_;
    rhor.r_direction = ray.direction - 2 * ray.direction.dot(rhor.hit_normal) * rhor.hit_normal; // it's already normalized
    rhor.point_color = color_;
    rhor.depth = t;
    return;
  }

  rhor.hit_point = Vec3f(0, 0, 0);
  rhor.hit_normal = Vec3f(0, 0, 0);
  rhor.r_direction = Vec3f(0, 0, 0);
  rhor.point_color = Vec3f(0, 0, 0);
  rhor.depth = -1;
}
void Plane::GetBoundingBox(Vec3f &AA, Vec3f &BB) {
  AA = AA;
  BB = BB;
}

Triangle::Triangle(const Vertex &A, const Vertex &B, const Vertex &C, Vec3f color)
  : GeometryObject("triangle", color)
  , A_(A)
  , B_(B)
  , C_(C) {
  AA_[0] = std::min(std::min(A_.Position[0], B_.Position[0]), C_.Position[0]);
  AA_[1] = std::min(std::min(A_.Position[1], B_.Position[1]), C_.Position[1]);
  AA_[2] = std::min(std::min(A_.Position[2], B_.Position[2]), C_.Position[2]);

  BB_[0] = std::max(std::max(A_.Position[0], B_.Position[0]), C_.Position[0]);
  BB_[1] = std::max(std::max(A_.Position[1], B_.Position[1]), C_.Position[1]);
  BB_[2] = std::max(std::max(A_.Position[2], B_.Position[2]), C_.Position[2]);

  baryCenter_ = (A_.Position + B_.Position + C_.Position) / 3.0f;
  eAB_ = B_.Position - A_.Position;
  eAC_ = C_.Position - A_.Position;
}
void Triangle::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
  Vec3f s = ray.s_point - A_.Position;
  Vec3f d = ray.direction;

  float denominator = d.cross(eAC_).dot(eAB_);

  float b1 = d.cross(eAC_).dot(s) / denominator;
  float b2 = eAB_.cross(d).dot(s) / denominator;
  float t = eAB_.cross(eAC_).dot(s) / denominator;
  if (t > MYEPSILON && b1 > -MYEPSILON && b2 > -MYEPSILON && b1 + b2 < 1 + MYEPSILON) {
    rhor.hit_point = ray.GetPoint(t);
    rhor.hit_normal = ((1 - b1 - b2) * A_.Normal + b1 * B_.Normal + b2 * C_.Normal).normalized();
    //rhor.hit_normal = eAB_.cross(eAC_).normalized();
    rhor.r_direction = ray.direction - 2 * ray.direction.dot(rhor.hit_normal) * rhor.hit_normal; // it's already normalized
    rhor.point_color = color_;
    rhor.depth = t;
    return;
  }

  rhor.hit_point = Vec3f(0, 0, 0);
  rhor.hit_normal = Vec3f(0, 0, 0);
  rhor.r_direction = Vec3f(0, 0, 0);
  rhor.point_color = Vec3f(0, 0, 0);
  rhor.depth = -1;
}
void Triangle::GetBoundingBox(Vec3f &AA, Vec3f &BB) {
  AA = AA;
  BB = BB;
}

Mesh::Mesh(const std::vector<Triangle::Vertex> &vertices, const std::vector<int> &faces, const Vec3f &color)
  : GeometryObject("Mesh", color)
  , sKDT_(nullptr) {
  faceTriangles_.clear();
  for (int i = 0; i < faces.size(); i += 3) {
    faceTriangles_.emplace_back(new Triangle(vertices[faces[i]], vertices[faces[i + 1]], vertices[faces[i + 2]], color));
  }

  sKDT_ = new KDTree(faceTriangles_);
}
Mesh::~Mesh() {
  for (std::vector<Triangle*>::iterator i = faceTriangles_.begin(); i != faceTriangles_.end(); ++i) {
    SafeDelete(*i);
  }
  SafeDelete(sKDT_);
}
void Mesh::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
  KDTree::TreeNode* node = sKDT_->rootNode;
  HitTree(ray, node, rhor);
}
void Mesh::HitTree(const Ray &ray, KDTree::TreeNode* node, RayHitObjectRecord &rhor) {
  if (RayHitAABB(ray, node->AA, node->BB)) {
    if (node->triangleIdx.size() > 0) {
      RayHitObjectRecord rhorT;
      for (std::vector<int>::iterator i = node->triangleIdx.begin(); i != node->triangleIdx.end(); ++i) {
        faceTriangles_[*i]->RayIntersection(ray, rhorT);
        if (rhorT.depth > MYEPSILON && (rhor.depth > rhorT.depth || rhor.depth < MYEPSILON)) {
          rhor = rhorT;
        }
      }
    }
    else {
      HitTree(ray, node->lChild, rhor);
      HitTree(ray, node->rChild, rhor);
    }
  }
}

Model::Model(const std::string &modelPath, const Vec3f &color)
  : GeometryObject("Model", color) {
  srand(time(0));

  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
    aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_SplitLargeMeshes | aiProcess_OptimizeMeshes);

  if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    //std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return;
  }

  meshes_.clear();
  processNode(scene->mRootNode, scene);
}
Model::~Model() {
  for (std::vector<Mesh*>::iterator i = meshes_.begin(); i != meshes_.end(); ++i) {
    SafeDelete(*i);
  }
}
void Model::RayIntersection(const Ray &Ray, RayHitObjectRecord &rhor) {
  RayHitObjectRecord rhorT;
  for (unsigned int i = 0; i < meshes_.size(); ++i) {
    meshes_[i]->RayIntersection(Ray, rhorT);
    if (rhorT.depth > MYEPSILON && (rhor.depth > rhorT.depth || rhor.depth < MYEPSILON)) {
      rhor = rhorT;
    }
  }
}
void Model::processNode(aiNode* node, const aiScene* scene) {
  // Process all the node's meshes_ (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes_.emplace_back(processMesh(mesh, scene));
  }
  // Then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    processNode(node->mChildren[i], scene);
  }
}
Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene) {
  std::vector<Triangle::Vertex> vertices;
  std::vector<int> faces;

  vertices.resize(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    Triangle::Vertex vertex;
    // Process vertex positions, normals and texture coordinates

    Vec3f vector;
    // Positions
    vector << mesh->mVertices[i].x,
      mesh->mVertices[i].y,
      mesh->mVertices[i].z;
    vertex.Position = vector;
    // Normals
    vector << mesh->mNormals[i].x,
      mesh->mNormals[i].y,
      mesh->mNormals[i].z;
    vertex.Normal = vector;

    vertices[i] = vertex;
  }
  // Process faces
  faces.resize(3 * mesh->mNumFaces);
  for (unsigned int i = 0, i1 = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];
    // Retrieve all indices of the face and store them in the indices vector
    for (unsigned int j = 0; j < face.mNumIndices; ++j) {
      faces[i1++] = face.mIndices[j];
    }
  }

  //return new Mesh(vertices, faces, color);
  return new Mesh(vertices, faces, Vec3f(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX));
}

}
