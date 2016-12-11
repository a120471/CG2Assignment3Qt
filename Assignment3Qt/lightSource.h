#pragma once

#include <vector>
#include <string>

#include <glm/gtc/type_ptr.hpp>

#include "quadTree.h"
#include "geometryObject.h"

// we normalize light color in unit voxel space, may be too big?!
class LightBase
{
public:
	virtual ~LightBase(){};

	virtual void GetLight(glm::vec3 sPoint, std::vector<glm::vec3> &colorList, std::vector<float> &disList, std::vector<glm::vec3> &lightDirList) = 0;

	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) = 0;
};

// a voxel point light, will a vexel be toooooo big?
class PointLight : public LightBase
{
public:
	PointLight(glm::vec3 pos, glm::vec3 color);
	virtual ~PointLight(){};

	virtual void GetLight(glm::vec3, std::vector<glm::vec3>&, std::vector<float>&, std::vector<glm::vec3>&) override;

	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) override;

private:
	glm::vec3 color, pos;
};

class AreaLight : public LightBase
{
public:
	AreaLight(glm::vec2 areaWH, glm::ivec2 resoWH, glm::vec3 pos, glm::vec3 totalColor, glm::vec3 dRight, glm::vec3 dDown);
	virtual ~AreaLight();

	virtual void GetLight(glm::vec3, std::vector<glm::vec3>&, std::vector<float>&, std::vector<glm::vec3>&) override;

	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) override;

private:
	glm::vec3 unitColor, pos, normal;
	float w, h;
	std::vector<PointLight*> pointSamples;
};

class SquareMap : public LightBase
{
public:
	SquareMap(glm::vec3 **data, int n, glm::vec3 ulCorner, glm::vec3 dRight, glm::vec3 dDown, float size);
	virtual ~SquareMap();

	virtual void GetLight(glm::vec3, std::vector<glm::vec3>&, std::vector<float>&, std::vector<glm::vec3>&) override;

	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) override;

private:
	glm::vec3 **data;
	int n;
	glm::vec3 ulCorner, dRight, dDown;
	float size;
	glm::vec3 normal;
	float D;
	QuadTree* quadT;
	std::vector<AreaLight*> lightSamples;
};

class CubeMap : public LightBase
{
public:
	CubeMap(std::string cubeMapPath, float size);
	CubeMap(std::string cubeMapPath[]);
	virtual ~CubeMap();

	virtual void GetLight(glm::vec3, std::vector<glm::vec3>&, std::vector<float>&, std::vector<glm::vec3>&) override;

	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) override;

private:
	float *loadImage;
	int width, height, dimension, N;
	SquareMap *top;
	SquareMap *bottom;
	SquareMap *left;
	SquareMap *right;
	SquareMap *forward;
	SquareMap *backward;

	void ExtractSquareMap(SquareMap *&sm, int smIdx, int rowIdx, int colIdx, bool rowInverse, bool colInverse, float size);
};
