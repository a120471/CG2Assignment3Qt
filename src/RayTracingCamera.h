#pragma once

#include <vector>
#include <glm/gtc/type_ptr.hpp>

namespace ray_tracing {

class Ray {
public:
  Ray(const glm::vec3 &origin_point, const glm::vec3 &dir);
  ~Ray() = default;

  // return a point on this ray
  inline glm::vec3 GetPoint(float t) const {
    return s_point + direction * t;
  }

  glm::vec3 s_point;
  glm::vec3 direction;
};

class RayTracingCamera {
public:
  RayTracingCamera(const glm::vec3 &pos = glm::vec3(0, 0, 0),
    const glm::vec3 &lookat = glm::vec3(0, 0, -1),
    const glm::vec3 &up = glm::vec3(0, 1, 0),
    int multi_sampling = 1);
  ~RayTracingCamera() = default;

  glm::vec3 getPos() {return pos_;}
  glm::vec3 getFront() {return front_;}
  glm::vec3 getUp() {return up_;}
  void SetCameraPos(glm::vec3 pos, glm::vec3 lookat, glm::vec3 up = glm::vec3(0, 1, 0))  {
    pos_ = pos;
    front_ = normalize(lookat - pos_);
    up_ = normalize(up_);

    ir_ = normalize(cross(front_, up_));
    id_ = normalize(cross(front_, ir_));
  }

  const glm::uvec2 &GetResolution();
  void SetResolution(const glm::uvec2 &resolution);
  const glm::vec3 &GetWHF();
  void SetWHF(const glm::vec3 &whf);
  glm::vec3 GetP() {return p_;}
  void SetP(glm::vec3 p) {p_ = p;}
  int GetRayNumEachPixel();

  // compute the list of rays emit from pixel (i, j)
  void GenerateRay(int row, int col, std::vector<Ray> &rays);

private:
  // camera location and orientation
  glm::vec3 pos_, front_, up_;
  // number of pixels
  glm::uvec2 resolution_;
  // image width, image height, local length
  glm::vec3 whf_;
  // image center position
  glm::vec3 p_;

  // image right direction, image down direction
  glm::vec3 ir_, id_;
  bool has_pixel_size_ = false;
  float pixel_width_, pixel_height_;

  // decide how many rays are generated in each pixel
  int multi_samling_level_;
};

}
