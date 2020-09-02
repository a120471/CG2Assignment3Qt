#include "RayTracingCamera.h"

namespace ray_tracing {

Ray::Ray(const glm::vec3 &origin_point, const glm::vec3 &dir)
	: s_point(origin_point)
	, direction(glm::normalize(dir)) {
}


RayTracingCamera::RayTracingCamera(
	const glm::vec3 &pos,
  const glm::vec3 &lookat,
  const glm::vec3 &up,
	int multi_samling_level_)
	: pos_(pos) {
	front_ = glm::normalize(lookat - pos);
	up_ = glm::normalize(up);

	ir_ = glm::normalize(glm::cross(front_, up_));
	id_ = glm::normalize(glm::cross(front_, ir_));
	has_pixel_size_ = false;

	multi_samling_level_ = (int)sqrt(multi_samling_level_);
}

// row and col start from 0
void RayTracingCamera::GenerateRay(int row, int col, std::vector<Ray*> &rays) {
	if (!has_pixel_size_) {
		has_pixel_size_ = true;
		pixel_width_ = whf_.x / resolution_.x;
		pixel_height_ = whf_.y / resolution_.y;
	}

	glm::vec3 colOffset = glm::vec3(ir_ * (col - (resolution_.x - 1.0f) / 2)) * pixel_width_;
	glm::vec3 rowOffset = glm::vec3(id_ * (row - (resolution_.y - 1.0f) / 2)) * pixel_height_;
	glm::vec3 ePoint = p_ + colOffset + rowOffset;

	rays.resize(multi_samling_level_ * multi_samling_level_);
	float step = 1.0f / (multi_samling_level_ + 1);
	int loop = 0;
	for (int i = 1; i <= multi_samling_level_; ++i) {
		for (int j = 1; j <= multi_samling_level_; ++j) {
			glm::vec3 colOffsetLocal = glm::vec3(ir_ * (i * step - 0.5f)) * pixel_width_;
			glm::vec3 rowOffsetLocal = glm::vec3(id_ * (j * step - 0.5f)) * pixel_height_;

			glm::vec3 directionLocal = normalize(ePoint + colOffsetLocal + rowOffsetLocal - pos_);
			rays[loop++] = new Ray(pos_, directionLocal);
		}
	}
}

const glm::uvec2 &RayTracingCamera::GetResolution() {
	return resolution_;
}

void RayTracingCamera::SetResolution(const glm::uvec2 &resolution) {
	resolution_ = resolution;
	has_pixel_size_ = false;
}

const glm::vec3 &RayTracingCamera::GetWHF() {
	return whf_;
}

void RayTracingCamera::SetWHF(const glm::vec3 &whf) {
	whf_ = whf;
	has_pixel_size_ = false;
}

int RayTracingCamera::GetRayNumEachPixel() {
	return multi_samling_level_ * multi_samling_level_;
}

}
