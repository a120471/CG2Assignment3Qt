#include "lightSource.h"

#include "Utils.h"

// stb_image, Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/gtc/type_ptr.hpp>

#pragma region PointLight
PointLight::PointLight(glm::vec3 pos, glm::vec3 color)
	: color(color)
	, pos(pos)
{
}
void PointLight::GetLight(glm::vec3 sPoint, std::vector<glm::vec3> &colorList, std::vector<float> &disList, std::vector<glm::vec3> &lightDirList)
{
	colorList.push_back(this->color);
	disList.push_back(length(this->pos - sPoint));
	lightDirList.push_back(normalize(this->pos - sPoint));
}
void PointLight::RayIntersection(RayClass* ray, RayHitObjectRecord &rhor)
{
	rhor.hitPoint = glm::vec3(0, 0, 0);
	rhor.hitNormal = glm::vec3(0, 0, 0);
	rhor.rDirection = glm::vec3(0, 0, 0);
	rhor.pointColor = glm::vec3(0, 0, 0);
	rhor.depth = -1;
}
#pragma endregion

#pragma region AreaLight
AreaLight::AreaLight(glm::vec2 areaWH, glm::ivec2 resoWH, glm::vec3 pos, glm::vec3 totalColor, glm::vec3 dRight, glm::vec3 dDown)
	: unitColor(totalColor / static_cast<float>(resoWH[0] * resoWH[1]))
	, pos(pos)
	, w(areaWH[0])
	, h(areaWH[1])
{
	normal = glm::normalize(glm::cross(dDown, dRight));

	float numF = totalColor.x / UNITAREASAMPLECOLOR;
	numF = glm::max(numF, totalColor.y / UNITAREASAMPLECOLOR);
	numF = glm::max(numF, totalColor.z / UNITAREASAMPLECOLOR);
	int num = static_cast<int>(numF);

	std::vector<glm::vec2> point;
	BestCandidateAlgorithm(point, num, areaWH[0], areaWH[1]);

	for (int i = 0; i < num; i++)
	{
		glm::vec3 pointPos;
		pointPos = pos + dRight * point[i].x + dDown * point[i].y;
		pointSamples.push_back(new PointLight(pointPos, unitColor / (float)num));
	}
}
AreaLight::~AreaLight()
{
	for (std::vector<PointLight*>::iterator i = pointSamples.begin(); i != pointSamples.end(); i++)
	{
		safe_delete(*i);
	}
}
void AreaLight::GetLight(glm::vec3 sPoint, std::vector<glm::vec3> &colorList, std::vector<float> &disList, std::vector<glm::vec3> &lightDirList)
{
	for (int i = 0; i < pointSamples.size(); i++)
	{
		pointSamples[i]->GetLight(sPoint, colorList, disList, lightDirList);
	}
}
void AreaLight::RayIntersection(RayClass* ray, RayHitObjectRecord &rhor)
{
	rhor.hitPoint = glm::vec3(0, 0, 0);
	rhor.hitNormal = glm::vec3(0, 0, 0);
	rhor.rDirection = glm::vec3(0, 0, 0);
	rhor.pointColor = glm::vec3(0, 0, 0);
	rhor.depth = -1;
}
#pragma endregion

#pragma region SquareMap
SquareMap::SquareMap(glm::vec3 **data, int n, glm::vec3 ulCorner, glm::vec3 dRight, glm::vec3 dDown, float size)
	: data(data)
	, n(n)
	, ulCorner(ulCorner)
	, dRight(dRight)
	, dDown(dDown)
	, size(size)
	, quadT(NULL)
{
	this->normal = normalize(cross(dDown, dRight));
	this->D = -dot(ulCorner, normal);

	this->quadT = new QuadTree(data, n, size);

	for (unsigned int i = 0; i < quadT->areaColor.size(); i++)
	{
		glm::vec3 pos;
		pos = ulCorner + (dRight * (float)quadT->posRC[i][1] + dDown * (float)quadT->posRC[i][0]) * (size / n);
		this->lightSamples.push_back(new AreaLight(quadT->sizeWH[i], quadT->resoWH[i], pos, quadT->areaColor[i], dRight, dDown));
	}
}
SquareMap::~SquareMap()
{
	for (int i = 0; i < n; i++)
	{
		delete[] data[i];
		data[i] = NULL;
	}
	delete[] data;
	data = NULL;

	for (std::vector<AreaLight*>::iterator i = lightSamples.begin(); i != lightSamples.end(); i++)
	{
		safe_delete(*i);
	}
	safe_delete(quadT);
}
void SquareMap::GetLight(glm::vec3 sPoint, std::vector<glm::vec3> &colorList, std::vector<float> &disList, std::vector<glm::vec3> &lightDirList)
{
	for (unsigned int i = 0; i < this->lightSamples.size(); i++)
	{
		this->lightSamples[i]->GetLight(sPoint, colorList, disList, lightDirList);
	}
}
void SquareMap::RayIntersection(RayClass* ray, RayHitObjectRecord &rhor)
{
	glm::vec3 sp = ray->sPoint;
	glm::vec3 d = ray->direction;
	
	float denominator = dot(this->normal, d);
	float numerator = -this->D - dot(this->normal, sp);

	float t = numerator / denominator;
	while (t > MYEPSILON)
	{
		glm::vec3 hitPoint = ray->getPoint(t);
		for (int i = 0; i < 4; i++)
		{
			glm::vec3 &e = glm::vec3();
			glm::vec3 n;
			glm::vec3 &n_ = glm::vec3();

			switch (i)
			{
			case 0:
				e = this->dDown;
				n_ = hitPoint - ulCorner;
				break;
			case 1:
				e = this->dRight;
				n_ = hitPoint - ulCorner - dDown * size;
				break;
			case 2:
				e = -this->dDown;
				n_ = hitPoint - ulCorner - dDown * size - dRight * size;
				break;
			case 3:
				e = -this->dRight;
				n_ = hitPoint - ulCorner - dRight * size;
				break;
			}
			n = cross(this->normal, e);

			if (dot(n, n_) < 0)
			{
				rhor.hitPoint = glm::vec3(0, 0, 0);
				rhor.hitNormal = glm::vec3(0, 0, 0);
				rhor.rDirection = glm::vec3(0, 0, 0);
				rhor.pointColor = glm::vec3(0, 0, 0);
				rhor.depth = -1;
				return;
			}
		}

		rhor.hitPoint = ray->getPoint(t);
		rhor.hitNormal = this->normal;
		rhor.rDirection = ray->direction - 2 * dot(ray->direction, rhor.hitNormal) * rhor.hitNormal; // it's already normalized
		
		float wCoord = dot((rhor.hitPoint - ulCorner), dRight) / size * n;
		float hCoord = dot((rhor.hitPoint - ulCorner), dDown) / size * n;

		if ((int)wCoord < 0) wCoord = 0.0f;
		if ((int)wCoord >= n - 2) wCoord = (float)(n - 2);
		if ((int)hCoord < 0) hCoord = 0.0f;
		if ((int)hCoord >= n - 2) hCoord = (float)(n - 2);

		float ww1 = wCoord - floor(wCoord);
		float ww2 = 1.0f - ww1;
		float wh1 = hCoord - floor(hCoord);
		float wh2 = 1.0f - wh1;
		
		glm::vec3 color = data[(int)hCoord][(int)wCoord] * ww2 * wh2 +
			data[(int)hCoord][(int)wCoord + 1] * ww1 * wh2 +
			data[(int)hCoord + 1][(int)wCoord] * ww2 * wh1 +
			data[(int)hCoord + 1][(int)wCoord + 1] * ww1 * wh1;

		rhor.pointColor = color;
		rhor.depth = t;
		return;
	}

	rhor.hitPoint = glm::vec3(0, 0, 0);
	rhor.hitNormal = glm::vec3(0, 0, 0);
	rhor.rDirection = glm::vec3(0, 0, 0);
	rhor.pointColor = glm::vec3(0, 0, 0);
	rhor.depth = -1;
}
#pragma endregion

#pragma region CubeMap
CubeMap::CubeMap(std::string cubeMapPath, float size)
	: loadImage(NULL)
{
	loadImage = stbi_loadf(cubeMapPath.c_str(), &width, &height, &dimension, 0);
	N = width > height ? width / 4 : height / 4;
	
	if (dimension != 3)	
	{
		/* should be forbidden */ 
		exit(-1);
	}

	ExtractSquareMap(top, 0, 0, 1, false, false, size);		// top
	ExtractSquareMap(bottom, 1, 2, 1, false, false, size);	// bottom
	ExtractSquareMap(left, 2, 1, 0, false, false, size);	// left
	ExtractSquareMap(right, 3, 1, 2, false, false, size);	// right
	ExtractSquareMap(forward, 4, 1, 1, false, false, size);	// forward
	if (width > height)
		ExtractSquareMap(backward, 5, 1, 3, false, false, size);// backward
	else
		ExtractSquareMap(backward, 5, 3, 1, true, true, size);	// backward

	stbi_image_free(loadImage);
}
CubeMap::CubeMap(std::string cubeMapPath[])
{
	/* to do */	
}
CubeMap::~CubeMap()
{
	safe_delete(top);
	safe_delete(bottom);
	safe_delete(left);
	safe_delete(right);
	safe_delete(forward);
	safe_delete(backward);
}
void CubeMap::ExtractSquareMap(SquareMap *&sm, int idx, int rowIdx, int colIdx, bool rowInverse, bool colInverse, float size)
{
	glm::vec3 **area = new glm::vec3*[N];
	for (int i = 0; i < N; i++)
		area[i] = new glm::vec3[N];
	// do not delete area, we delete it in ~SquareMap()

	int rI, imageI;
	rI = rowInverse ? rowIdx * N + N - 1 : rowIdx * N;
	rI *= width * dimension;
	for (int r = 0; r < N; r++)
	{
		if (!colInverse)
		{
			imageI = rI + colIdx * N * dimension;
			for (int c = 0; c < N; c++)
			{
				area[r][c] = glm::vec3(loadImage[imageI], loadImage[imageI + 1], loadImage[imageI + 2]);
				imageI += 3;
			}
		}
		else
		{
			imageI = rI + (colIdx + 1) * N * dimension - 1;
			for (int c = 0; c < N; c++)
			{
				area[r][c] = glm::vec3(loadImage[imageI - 2], loadImage[imageI - 1], loadImage[imageI]);
				imageI -= 3;
			}
		}

		rI += rowInverse ? -width * dimension : width * dimension;
	}

	// to calculate area light's pos
	glm::vec3 ulCorner, dR, dD;
	float size_2 = size / 2.0f;
	switch (idx)
	{
	case 0: // top
		ulCorner = glm::vec3(-size_2, size_2, size_2);
		dR = glm::vec3(1, 0, 0);
		dD = glm::vec3(0, 0, -1);
		break;
	case 1: // bottom
		ulCorner = glm::vec3(-size_2, -size_2, -size_2);
		dR = glm::vec3(1, 0, 0);
		dD = glm::vec3(0, 0, 1);
		break;
	case 2: // left
		ulCorner = glm::vec3(-size_2, size_2, size_2);
		dR = glm::vec3(0, 0, -1);
		dD = glm::vec3(0, -1, 0);
		break;
	case 3: // right
		ulCorner = glm::vec3(size_2, size_2, -size_2);
		dR = glm::vec3(0, 0, 1);
		dD = glm::vec3(0, -1, 0);
		break;
	case 4: // forward
		ulCorner = glm::vec3(-size_2, size_2, -size_2);
		dR = glm::vec3(1, 0, 0);
		dD = glm::vec3(0, -1, 0);
		break;
	case 5: // backward
		ulCorner = glm::vec3(size_2, size_2, size_2);
		dR = glm::vec3(-1, 0, 0);
		dD = glm::vec3(0, -1, 0);
		break;
	}

	sm = new SquareMap(area, N, ulCorner, dR, dD, size);
}
void CubeMap::GetLight(glm::vec3 sPoint, std::vector<glm::vec3> &colorList, std::vector<float> &disList, std::vector<glm::vec3> &lightDirList)
{
	this->top->GetLight(sPoint, colorList, disList, lightDirList);
	// this->bottom->GetLight(sPoint, colorList, disList, lightDirList);
	this->left->GetLight(sPoint, colorList, disList, lightDirList);
	this->right->GetLight(sPoint, colorList, disList, lightDirList);
	this->forward->GetLight(sPoint, colorList, disList, lightDirList);
	this->backward->GetLight(sPoint, colorList, disList, lightDirList);
}
void CubeMap::RayIntersection(RayClass* ray, RayHitObjectRecord &rhor)
{
	RayHitObjectRecord rhorT;
	for (int i = 0; i < 4; i++)
	{
		SquareMap *p;
		if (i == 0) p = this->top;
		else if (i == 1) p = this->left;
		else if (i == 2) p = this->right;
		else if (i == 3) p = this->forward;
		else if (i == 4) p = this->backward;

		p->RayIntersection(ray, rhorT);
		if (rhorT.depth > MYEPSILON && (rhor.depth > rhorT.depth || rhor.depth < MYEPSILON))
			rhor = rhorT;
	}
}
#pragma endregion