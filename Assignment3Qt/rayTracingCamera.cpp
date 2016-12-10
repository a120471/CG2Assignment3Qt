#include "rayTracingCamera.h"

RayClass::RayClass(glm::vec3 sPoint, glm::vec3 directoin)
	: sPoint(sPoint)
{
	this->direction = normalize(directoin);
}


RayTracingCameraClass::RayTracingCameraClass(glm::vec3 pos, glm::vec3 lookat, glm::vec3 up, int antiAliasingLevel)
	: pos(pos)
{
	this->front = normalize(lookat - pos);
	this->up = normalize(up);

	this->ir = normalize(cross(this->front, this->up));
	this->id = normalize(cross(this->front, this->ir));
	this->hasPixelSize = false;

	this->antiAliasingLevel = (int)sqrt(antiAliasingLevel);
}

// row and col start from 0
void RayTracingCameraClass::GenerateRay(int row, int col, std::vector<RayClass*> &rays)
{
	if (!hasPixelSize)
	{
		this->hasPixelSize = true;
		this->pixelWidth = this->iw / this->w;
		this->pixelHeight = this->ih / this->h;
	}

	glm::vec3 colOffset = glm::vec3(this->ir * (col - (this->w - 1.0f) / 2)) * this->pixelWidth;
	glm::vec3 rowOffset = glm::vec3(this->id * (row - (this->h - 1.0f) / 2)) * this->pixelHeight;
	glm::vec3 ePoint = this->p + colOffset + rowOffset;

	rays.resize(this->antiAliasingLevel * this->antiAliasingLevel);
	float step = 1.0f / (this->antiAliasingLevel + 1);
	int loop = 0;
	for (int i = 1; i <= this->antiAliasingLevel; i++)
	{
		for (int j = 1; j <= this->antiAliasingLevel; j++)
		{
			glm::vec3 colOffsetLocal = glm::vec3(this->ir * (i * step - 0.5f)) * this->pixelWidth;
			glm::vec3 rowOffsetLocal = glm::vec3(this->id * (j * step - 0.5f)) * this->pixelHeight;

			glm::vec3 directionLocal = normalize(ePoint + colOffsetLocal + rowOffsetLocal - this->pos);
			rays[loop++] = new RayClass(this->pos, directionLocal);
		}
	}
}
