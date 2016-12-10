// define each geometry type
#pragma once

#include <string>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>

#include "rayTracingCamera.h"
#include "spaceKDTree.h"
#include "Utils.h"

// saved data of each hit point and reflection direction
struct RayHitObjectRecord
{
	RayHitObjectRecord(float depth = -1)
	{
		this->depth = depth;
	}

	RayHitObjectRecord& operator=(const RayHitObjectRecord& rhor)
	{
		if (this == &rhor) 
			return *this;
		
		this->depth = rhor.depth;
		this->hitPoint = rhor.hitPoint;
		this->hitNormal = rhor.hitNormal;
		this->rDirection = rhor.rDirection;
		this->pointColor = rhor.pointColor;

		return *this;
	}

	float depth;
	glm::vec3 hitPoint;
	glm::vec3 hitNormal;
	glm::vec3 rDirection;
	glm::vec3 pointColor;
};


// parent class
class GeometryObject
{
public:
	GeometryObject(std::string typeName, glm::vec3 color);
	virtual ~GeometryObject(){};

	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) = 0;

	// we need to save the bounding box to accerlerate the ray hit test
	virtual void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) = 0;

	// void getMaterial();

	std::string typeName;
	glm::vec3 color;
	glm::vec3 AA, BB; // bounding box
	//glm::mat4 transformMatrix;
};

class Sphere : public GeometryObject
{
public:
	Sphere(glm::vec3 center, float radius, glm::vec3 color = glm::vec3(1, 1, 1));
	virtual ~Sphere(){};

	virtual void RayIntersection(RayClass* ray, RayHitObjectRecord &rhor) override;

	virtual void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override;

private:
	glm::vec3 center;
	float radius;
};

class Plane : public GeometryObject
{
public:
	Plane(float A, float B, float C, float D, glm::vec3 color = glm::vec3(1, 1, 1));
	virtual ~Plane(){};

	virtual void RayIntersection(RayClass* ray, RayHitObjectRecord &rhor) override;

	virtual void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override;

	glm::vec3 normal;

private:
	// Ax + By + Cz + D = 0;
	glm::vec3 ABC;
	float D;
};

class Triangle : public GeometryObject
{
public:
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
	};

	Triangle(const Vertex &A, const Vertex &B, const Vertex &C, glm::vec3 color = glm::vec3(1, 1, 1));
	virtual ~Triangle(){};

	virtual void RayIntersection(RayClass* ray, RayHitObjectRecord &rhor) override;

	virtual void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override;

	glm::vec3 baryCenter;

private:
	Vertex A, B, C;
	glm::vec3 eAB, eAC;
};

class Mesh : public GeometryObject
{
public:	
	Mesh(const std::vector<Triangle::Vertex> &vertices, const std::vector<int> &faces, glm::vec3 color = glm::vec3(1, 1, 1));
	virtual ~Mesh();

	virtual void RayIntersection(RayClass* Ray, RayHitObjectRecord &rhor) override;

	void HitTree(RayClass* ray, SpaceKDTree::TreeNode* node, RayHitObjectRecord &rhor);

	virtual void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override { /*to do*/ };

	inline static bool SortByX(const Triangle *t1, const Triangle *t2)
	{
		return t1->baryCenter[0] < t2->baryCenter[0];
	}
	inline static bool SortByY(const Triangle *t1, const Triangle *t2)
	{
		return t1->baryCenter[1] < t2->baryCenter[1];
	}
	inline static bool SortByZ(const Triangle *t1, const Triangle *t2)
	{
		return t1->baryCenter[2] < t2->baryCenter[2];
	}

private:
	std::vector<Triangle*> faceTriangles;
	SpaceKDTree* sKDT;
};

class Model : public GeometryObject
{
public:
	Model(std::string modelPath, glm::vec3 color = glm::vec3(1, 1, 1));
	virtual ~Model();

	virtual void RayIntersection(RayClass* Ray, RayHitObjectRecord &rhor) override;

	virtual void GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB) override { /*to do*/ };

private:
	void processNode(aiNode* node, const aiScene* scene);
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Mesh*> meshes;
};