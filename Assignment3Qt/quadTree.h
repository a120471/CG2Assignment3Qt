#pragma once

#include <vector>

#include <glm/gtc/type_ptr.hpp>

class QuadTree
{
public:
	QuadTree(glm::vec3 **data, int n, float size);
	~QuadTree();

	std::vector<glm::ivec2> posRC;
	std::vector<glm::vec2> sizeWH;
	std::vector<glm::ivec2> resoWH;
	std::vector<glm::vec3> areaColor;

private:
	void BuildQTree(int sR, int sC, int n);
	
	glm::dvec3 **sumMatrix;
	int N;
	float unitSize;
};