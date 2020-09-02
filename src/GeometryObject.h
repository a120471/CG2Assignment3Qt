// Define geometry types
#pragma once

#include <string>
#include <assimp/scene.h>
#include "KDTree.h"

namespace ray_tracing {

class Ray;

// saved data of each hit point and reflection direction
struct RayHitObjectRecord {
	RayHitObjectRecord(float depth = -1)
	{
		this->depth = depth;
	}

	RayHitObjectRecord& operator=(const RayHitObjectRecord& record) {
		if (this == &record)
			return *this;

		this->depth = record.depth;
		this->hit_point = record.hit_point;
		this->hit_normal = record.hit_normal;
		this->r_direction = record.r_direction;
		this->point_color = record.point_color;

		return *this;
	}

	float depth;
	glm::vec3 hit_point;
	glm::vec3 hit_normal;
	glm::vec3 r_direction;
	glm::vec3 point_color;
};


// Base class
class GeometryObject
{
public:
	GeometryObject(const std::string &typeName, const glm::vec3 &color);
	virtual ~GeometryObject() = default;

	virtual void RayIntersection(Ray *ray, RayHitObjectRecord &record) = 0;

	// we need to save the bounding box to accerlerate the ray hit test
	virtual void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) = 0;

	// void getMaterial();

	std::string typeName_;
	glm::vec3 color_;
	glm::vec3 AA_, BB_; // bounding box
	//glm::mat4 transformMatrix;
};

class Sphere : public GeometryObject {
public:
	Sphere(const glm::vec3 &center, float radius,
    const glm::vec3 &color = glm::vec3(1, 1, 1));

	void RayIntersection(Ray *ray, RayHitObjectRecord &record) override;

	void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override;

private:
	glm::vec3 center_;
	float radius_;
};

class Plane : public GeometryObject {
public:
	Plane(float A, float B, float C, float D,
    const glm::vec3 &color = glm::vec3(1, 1, 1));

	void RayIntersection(Ray *ray, RayHitObjectRecord &record) override;

	void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override;

	glm::vec3 normal_;

private:
	// Ax + By + Cz + D = 0;
	glm::vec3 ABC_;
	float D_;
};

class Triangle : public GeometryObject {
public:
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
	};

	Triangle(const Vertex &A, const Vertex &B, const Vertex &C,
    glm::vec3 color = glm::vec3(1, 1, 1));

	void RayIntersection(Ray *ray, RayHitObjectRecord &record) override;

	void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override;

	glm::vec3 baryCenter_;

private:
	Vertex A_, B_, C_;
	glm::vec3 eAB_, eAC_;
};

class Mesh : public GeometryObject {
public:
	Mesh(const std::vector<Triangle::Vertex> &vertices,
    const std::vector<int> &indices,
    const glm::vec3 &color = glm::vec3(1, 1, 1));
	virtual ~Mesh();

	void RayIntersection(Ray *Ray, RayHitObjectRecord &record) override;

	void HitTree(Ray *ray, KDTree::TreeNode *node, RayHitObjectRecord &record);

	void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override { /*to do*/ };

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
    const glm::vec3 &color = glm::vec3(1, 1, 1));
  virtual ~Model();

	void RayIntersection(Ray *Ray, RayHitObjectRecord &record) override;

	void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override { /*to do*/ };

private:
	void processNode(aiNode* node, const aiScene* scene);
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Mesh*> meshes_;
};

}
