#pragma once

#include <vector>
#include <glm/vec3.hpp>

// #include "quadTree.h"
// #include "geometryObject.h"

namespace ray_tracing {

class Ray;
struct RayHitObjectRecord;

// we normalize light color in unit voxel space, may be too big?!
class LightBase {
public:
	virtual ~LightBase() = default;

	virtual void GetLight(const glm::vec3 &s_point,
	  std::vector<glm::vec3> &colors,
	  std::vector<float> &distances,
		std::vector<glm::vec3> &light_dirs) = 0;

	virtual void RayIntersection(Ray *ray, RayHitObjectRecord &record) = 0;
};

// a voxel point light, will a voxel be toooooo big?
class PointLight : public LightBase {
public:
	PointLight(const glm::vec3 &pos, const glm::vec3 &color);

	void GetLight(const glm::vec3 &s_point,
	  std::vector<glm::vec3> &colors,
		std::vector<float> &distances,
		std::vector<glm::vec3> &light_dirs) override;

	void RayIntersection(Ray *ray, RayHitObjectRecord &record) override;

private:
	glm::vec3 pos_;
	glm::vec3 color_;
};

// class AreaLight : public LightBase
// {
// public:
// 	AreaLight(glm::vec2 areaWH, glm::ivec2 resoWH, glm::vec3 pos, glm::vec3 totalColor, glm::vec3 dRight, glm::vec3 dDown);
// 	virtual ~AreaLight();

// 	virtual void GetLight(glm::vec3, std::vector<glm::vec3>&, std::vector<float>&, std::vector<glm::vec3>&) override;

// 	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) override;

// private:
// 	glm::vec3 unitColor, pos, normal;
// 	float w, h;
// 	std::vector<PointLight*> pointSamples;
// };

// class SquareMap : public LightBase
// {
// public:
// 	SquareMap(glm::vec3 **data, int n, glm::vec3 ulCorner, glm::vec3 dRight, glm::vec3 dDown, float size);
// 	virtual ~SquareMap();

// 	virtual void GetLight(glm::vec3, std::vector<glm::vec3>&, std::vector<float>&, std::vector<glm::vec3>&) override;

// 	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) override;

// private:
// 	glm::vec3 **data;
// 	int n;
// 	glm::vec3 ulCorner, dRight, dDown;
// 	float size;
// 	glm::vec3 normal;
// 	float D;
// 	QuadTree* quadT;
// 	std::vector<AreaLight*> lightSamples;
// };

// class CubeMap : public LightBase
// {
// public:
// 	CubeMap(std::string cubeMapPath, float size);
// 	CubeMap(std::string cubeMapPath[]);
// 	virtual ~CubeMap();

// 	virtual void GetLight(glm::vec3, std::vector<glm::vec3>&, std::vector<float>&, std::vector<glm::vec3>&) override;

// 	virtual void RayIntersection(RayClass*, RayHitObjectRecord&) override;

// private:
// 	float *loadImage;
// 	int width, height, dimension, N;
// 	SquareMap *top;
// 	SquareMap *bottom;
// 	SquareMap *left;
// 	SquareMap *right;
// 	SquareMap *forward;
// 	SquareMap *backward;

// 	void ExtractSquareMap(SquareMap *&sm, int smIdx, int rowIdx, int colIdx, bool rowInverse, bool colInverse, float size);
// };

}
