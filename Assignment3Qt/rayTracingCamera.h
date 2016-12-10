#pragma once

#include <vector>

#include <glm/gtc/type_ptr.hpp>

class RayClass
{
public:
	RayClass(glm::vec3 sPoint, glm::vec3 directoin);
	~RayClass(){ /*printf("A Ray has just been destroyed :-(\n");*/ };

	// return a point on this ray
	inline glm::vec3 getPoint(float t) { return this->sPoint + this->direction * t; }

	glm::vec3 sPoint, direction;
};

class RayTracingCameraClass
{
public:
	RayTracingCameraClass(glm::vec3 pos = glm::vec3(0, 0, 0), glm::vec3 lookat = glm::vec3(0, 0, -1), glm::vec3 up = glm::vec3(0, 1, 0), int antiAliasingLevel = 1);
	~RayTracingCameraClass(){};

	glm::vec3 getPos()		{ return this->pos; };
	glm::vec3 getFront()	{ return this->front; };
	glm::vec3 getUp()		{ return this->up; };
	void setCameraPos(glm::vec3 pos, glm::vec3 lookat, glm::vec3 up = glm::vec3(0, 1, 0))
	{
		this->pos = pos;
		this->front = normalize(lookat - pos);
		this->up = normalize(up);

		this->ir = normalize(cross(this->front, this->up));
		this->id = normalize(cross(this->front, this->ir));
	}

	int getW() 				{ return w; }
	void setW(int w)		{ this->w = w; this->hasPixelSize = false; }
	int getH()				{ return h; }
	void setH(int h)		{ this->h = h; this->hasPixelSize = false; }
	float getFL()			{ return fl; }
	void setFL(float fl)	{ this->fl = fl; this->hasPixelSize = false; }
	float getIW()			{ return iw; }
	void setIW(float iw)	{ this->iw = iw; this->hasPixelSize = false; }
	float getIH()			{ return ih; }
	void setIH(float ih)	{ this->ih = ih; this->hasPixelSize = false; }
	glm::vec3 getP()		{ return this->p; };
	void setP(glm::vec3 p)	{ this->p = p; }
	int getRayNumEachPixel(){ return this->antiAliasingLevel * this->antiAliasingLevel; }
	
	// compute the list of rays emit from pixel (i, j)
	void GenerateRay(int row, int col, std::vector<RayClass*> &rays);

private:
	// camera location and orientation
	glm::vec3 pos, front, up;
	// number of pixels
	int w, h;
	// camera focal length, image plane's width, image plane's height
	float fl, iw, ih;
	// image center position
	glm::vec3 p;

	// image right direction, image down direction
	glm::vec3 ir, id;
	bool hasPixelSize = false;
	float pixelWidth, pixelHeight;

	// decide how many rays are generated in each pixel
	int antiAliasingLevel;
};
