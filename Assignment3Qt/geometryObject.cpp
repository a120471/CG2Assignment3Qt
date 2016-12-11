#include "geometryObject.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <ctime>

#pragma region GeometryObject
GeometryObject::GeometryObject(std::string typeName, glm::vec3 color)
	: typeName(typeName)
	, color(color)
{
}
#pragma endregion

#pragma region Sphere
Sphere::Sphere(glm::vec3 center, float radius, glm::vec3 color)
	: GeometryObject("sphere", color)
	, center{ center }
	, radius{ radius }
{
	this->AA = this->center - radius;
	this->BB = this->center + radius;
}
void Sphere::RayIntersection(RayClass* ray, RayHitObjectRecord &rhor)
{
	glm::vec3 sc = ray->sPoint - this->center;
	glm::vec3 d = ray->direction;

	// At^2 + Bt + C = 0, solve t
	float A = dot(d, d);
	float B = 2 * dot(d, sc);
	float C = dot(sc, sc) - this->radius*this->radius;

	float det = B*B - 4 * A*C;
	if (det > MYEPSILON)
	{
		float t1 = (-B - sqrt(det)) / (2 * A);
		float t2 = (-B + sqrt(det)) / (2 * A);

		if (t1 > MYEPSILON)
		{
			rhor.hitPoint = ray->getPoint(t1);
			rhor.hitNormal = normalize(rhor.hitPoint - this->center);
			rhor.rDirection = ray->direction - 2 * dot(ray->direction, rhor.hitNormal) * rhor.hitNormal; // it's already normalized
			rhor.pointColor = this->color;
			rhor.depth = t1;
			return;
		}
		else if (t2 > MYEPSILON)
		{
			rhor.hitPoint = ray->getPoint(t2);
			rhor.hitNormal = normalize(rhor.hitPoint - this->center);
			rhor.rDirection = ray->direction - 2 * dot(ray->direction, rhor.hitNormal) * rhor.hitNormal; // it's already normalized
			rhor.pointColor = this->color;
			rhor.depth = t2;
			return;
		}
	}

	rhor.hitPoint = glm::vec3(0, 0, 0);
	rhor.hitNormal = glm::vec3(0, 0, 0);
	rhor.rDirection = glm::vec3(0, 0, 0);
	rhor.pointColor = glm::vec3(0, 0, 0);
	rhor.depth = -1;
}
void Sphere::GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB)
{
	AA = this->AA;
	BB = this->BB;
}

#pragma endregion

#pragma region Plane
Plane::Plane(float A, float B, float C, float D, glm::vec3 color)
	: GeometryObject("plane", color)
	, ABC{ glm::vec3(A, B, C) }
	, D{D}
{
	this->AA = glm::vec3(-MYINFINITE);
	this->BB = glm::vec3(MYINFINITE);

	this->normal = normalize(glm::vec3(A, B, C));
}
void Plane::RayIntersection(RayClass* ray, RayHitObjectRecord &rhor)
{
	glm::vec3 sp = ray->sPoint;
	glm::vec3 d = ray->direction;

	float denominator = dot(this->ABC, d);
	float numerator = -this->D - dot(this->ABC, sp);

	float t = numerator / denominator;
	if (t > MYEPSILON)
	{
		rhor.hitPoint = ray->getPoint(t);
		rhor.hitNormal = this->normal;
		rhor.rDirection = ray->direction - 2 * dot(ray->direction, rhor.hitNormal) * rhor.hitNormal; // it's already normalized
		rhor.pointColor = this->color;
		rhor.depth = t;
		return;
	}

	rhor.hitPoint = glm::vec3(0, 0, 0);
	rhor.hitNormal = glm::vec3(0, 0, 0);
	rhor.rDirection = glm::vec3(0, 0, 0);
	rhor.pointColor = glm::vec3(0, 0, 0);
	rhor.depth = -1;
}
void Plane::GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB)
{
	AA = this->AA;
	BB = this->BB;
}
#pragma endregion

#pragma region Triangle
Triangle::Triangle(const Vertex &A, const Vertex &B, const Vertex &C, glm::vec3 color)
	: GeometryObject("triangle", color)
	, A(A)
	, B(B)
	, C(C)
{
	AA[0] = glm::min(glm::min(A.Position[0], B.Position[0]), C.Position[0]);
	AA[1] = glm::min(glm::min(A.Position[1], B.Position[1]), C.Position[1]);
	AA[2] = glm::min(glm::min(A.Position[2], B.Position[2]), C.Position[2]);

	BB[0] = glm::max(glm::max(A.Position[0], B.Position[0]), C.Position[0]);
	BB[1] = glm::max(glm::max(A.Position[1], B.Position[1]), C.Position[1]);
	BB[2] = glm::max(glm::max(A.Position[2], B.Position[2]), C.Position[2]);

	this->baryCenter = (A.Position + B.Position + C.Position) / 3.0f;
	this->eAB = B.Position - A.Position;
	this->eAC = C.Position - A.Position;
}
void Triangle::RayIntersection(RayClass* ray, RayHitObjectRecord &rhor)
{
	glm::vec3 s = ray->sPoint - A.Position;
	glm::vec3 d = ray->direction;

	float denominator = dot(cross(d, eAC), eAB);

	float b1 = dot(cross(d, eAC), s) / denominator;
	float b2 = dot(cross(eAB, d), s) / denominator;
	float t = dot(cross(eAB, eAC), s) / denominator;
	if (t > MYEPSILON && b1 > -MYEPSILON && b2 > -MYEPSILON && b1 + b2 < 1 + MYEPSILON)
	{
		rhor.hitPoint = ray->getPoint(t);
		rhor.hitNormal = normalize((1 - b1 - b2) * A.Normal + b1 * B.Normal + b2 * C.Normal);
		//rhor.hitNormal = normalize(cross(eAB, eAC));
		rhor.rDirection = ray->direction - 2 * dot(ray->direction, rhor.hitNormal) * rhor.hitNormal; // it's already normalized
		rhor.pointColor = this->color;
		rhor.depth = t;
		return;
	}

	rhor.hitPoint = glm::vec3(0, 0, 0);
	rhor.hitNormal = glm::vec3(0, 0, 0);
	rhor.rDirection = glm::vec3(0, 0, 0);
	rhor.pointColor = glm::vec3(0, 0, 0);
	rhor.depth = -1;
}
void Triangle::GetBoundingBox(glm::vec3 &AA, glm::vec3 &BB)
{
	AA = this->AA;
	BB = this->BB;
}
#pragma endregion

#pragma region Mesh
Mesh::Mesh(const std::vector<Triangle::Vertex> &vertices, const std::vector<int> &faces, glm::vec3 color)
	: GeometryObject("Mesh", color)
	, sKDT(NULL)
{	
	this->faceTriangles.clear();
	for (int i = 0; i < faces.size(); i += 3)
	{
		this->faceTriangles.push_back(new Triangle(vertices[faces[i]], vertices[faces[i + 1]], vertices[faces[i + 2]], color));
	}

	this->sKDT = new SpaceKDTree(this->faceTriangles);
}
Mesh::~Mesh()
{
	for (std::vector<Triangle*>::iterator i = faceTriangles.begin(); i != faceTriangles.end(); i++)
	{
		safe_delete(*i);
	}
	safe_delete(sKDT);
}
void Mesh::RayIntersection(RayClass* ray, RayHitObjectRecord &rhor)
{
	SpaceKDTree::TreeNode* node = this->sKDT->rootNode;
	HitTree(ray, node, rhor);
}
void Mesh::HitTree(RayClass* ray, SpaceKDTree::TreeNode* node, RayHitObjectRecord &rhor)
{
	if (RayHitAABB(ray, node->AA, node->BB))
	{
		if (node->triangleIdx.size() > 0)
		{
			RayHitObjectRecord rhorT;
			for (std::vector<int>::iterator i = node->triangleIdx.begin(); i != node->triangleIdx.end(); i++)
			{
				this->faceTriangles[*i]->RayIntersection(ray, rhorT);
				if (rhorT.depth > MYEPSILON && (rhor.depth > rhorT.depth || rhor.depth < MYEPSILON))
				{
					rhor = rhorT;
				}
			}
		}
		else
		{
			HitTree(ray, node->lChild, rhor);
			HitTree(ray, node->rChild, rhor);
		}
	}
}
#pragma endregion

#pragma region Model
Model::Model(std::string modelPath, glm::vec3 color)
	: GeometryObject("Model", color)
{
	srand(time(0));

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | 
		aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_SplitLargeMeshes | aiProcess_OptimizeMeshes);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		//std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	this->meshes.clear();
	this->processNode(scene->mRootNode, scene);
}
Model::~Model()
{
	for (std::vector<Mesh*>::iterator i = meshes.begin(); i != meshes.end(); i++)
	{
		safe_delete(*i);
	}
}
void Model::RayIntersection(RayClass* Ray, RayHitObjectRecord &rhor)
{
	RayHitObjectRecord rhorT;
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i]->RayIntersection(Ray, rhorT);
		if (rhorT.depth > MYEPSILON && (rhor.depth > rhorT.depth || rhor.depth < MYEPSILON))
		{
			rhor = rhorT;
		}
	}
}
void Model::processNode(aiNode* node, const aiScene* scene)
{
	// Process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(this->processMesh(mesh, scene));
	}
	// Then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene);
	}
}
Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Triangle::Vertex> vertices;
	std::vector<int> faces;

	vertices.resize(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Triangle::Vertex vertex;
		// Process vertex positions, normals and texture coordinates

		glm::vec3 vector;
		// Positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// Normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		vertices[i] = vertex;
	}
	// Process faces
	faces.resize(3 * mesh->mNumFaces);
	for (unsigned int i = 0, i1 = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// Retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			faces[i1++] = face.mIndices[j];
	}

	//return new Mesh(vertices, faces, color);
	return new Mesh(vertices, faces, glm::vec3(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX));
}
#pragma endregion
