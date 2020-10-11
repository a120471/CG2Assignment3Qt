#include "LightSource.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "QuadTree.h"
#include "Utils.h"

namespace ray_tracing {

bool hasHDRLighting;

PointLight::PointLight(const Vec3f &pos, const Vec3f &color)
  : pos_(pos)
  , color_(color) {
}

void PointLight::AppendToLightInfo(const Vec3f &s_point,
  std::vector<LightInfoToPoint> &infos) const {
  infos.emplace_back(color_,
    (pos_ - s_point).normalized(),
    (pos_ - s_point).norm());
}

// RayHitRecord PointLight::RayIntersection(const Ray &ray) {
//   return RayHitRecord();
// }


AreaLight::AreaLight(const Vec3f &pos, const Vec3f &sum_color,
  const Vec2f &size, const Vec3f &right_dir, const Vec3f &down_dir) {

  Vec3f sample_num = sum_color / UNIT_SAMPLE_COLOR;
  uint32_t num = (uint32_t)sample_num.maxCoeff();

  // Sample num points on the area
  std::vector<Vec2f> points;
  GenerateCandidate(size, num, points);

  for (const auto &p : points) {
    Vec3f sample_pos = pos + right_dir * p(0) + down_dir * p(1);
    samples_.emplace_back(std::make_shared<PointLight>(
      sample_pos, sum_color / (float)num));
  }
}

void AreaLight::AppendToLightInfo(const Vec3f &s_point,
  std::vector<LightInfoToPoint> &infos) const {
  for (const auto &sample : samples_) {
    sample->AppendToLightInfo(s_point, infos);
  }
}

// RayHitRecord PointLight::RayIntersection(const Ray &ray) {
//   return RayHitRecord();
// }

// Mitchell’s best-candidate algorithm
// This implementation is neither efficient nor precise
void AreaLight::GenerateCandidate(const Vec2f &size,
    uint32_t num, std::vector<Vec2f> &points) {
  points.clear();
  if (num == 0) {
    return;
  }

  const int CANDIDATE_NUM = 2;
  uint32_t w = size(0);
  uint32_t h = size(1);

  srand(time(0));
  points.emplace_back(((float)rand() / RAND_MAX - 0.5f) * w,
    ((float)rand() / RAND_MAX - 0.5f) * h);
  for (auto i = 1; i < num; ++i) {
    float max_dis = 0;
    Vec2f best_candidate;
    for (auto j = 0; j < CANDIDATE_NUM; ++j) {
      Vec2f candidate(((float)rand() / RAND_MAX - 0.5f) * w,
        ((float)rand() / RAND_MAX - 0.5f) * h);
      for (const auto &p : points) {
        float dis = (p - candidate).norm();
        if (max_dis < dis) {
          max_dis = dis;
          best_candidate = candidate;
        }
      }
    }
    points.emplace_back(best_candidate);
  }
}


CubeMapFace::CubeMapFace(std::vector<std::vector<Vec3f>> &&data,
  const Vec3f &center, const Vec3f &right_dir,
  const Vec3f &down_dir, float size)
  : data_(std::move(data))
  , center_(center)
  , right_dir_(right_dir)
  , down_dir_(down_dir)
  , size_(size) {

  auto tree = std::make_shared<QuadTree>(data_, size_);
  const auto &node_infos = tree->GetNodeInfos();
  for (const auto &info : node_infos) {
    Vec3f pos = center_ + right_dir_ * info.pos(0) + down_dir_ * info.pos(1);
    light_samples_.emplace_back(std::make_shared<AreaLight>(pos,
      info.sum_color, info.size, right_dir_, down_dir_));
  }

  // todo, ?
  // normal_ = down_dir_.cross(right_dir_).normalized();
  // D = -ulCorner.dot(normal);
}

void CubeMapFace::AppendToLightInfo(const Vec3f &s_point,
  std::vector<LightInfoToPoint> &infos) const {
  for (const auto &sample : light_samples_) {
    sample->AppendToLightInfo(s_point, infos);
  }
}

// RayHitRecord CubeMapFace::RayIntersection(const Ray &ray) {
//   RayHitRecord record;
//   Vec3f sp = ray.sPoint;
//   Vec3f d = ray.direction;

//   float denominator = this->normal.dot(d);
//   float numerator = -this->D - this->normal.dot(sp);

//   float t = numerator / denominator;
//   while (t > MYEPSILON) {
//     Vec3f hit_point = ray.getPoint(t);
//     for (int i = 0; i < 4; ++i) {
//       Vec3f &e = Vec3f::Zero();
//       Vec3f n;
//       Vec3f &n_ = Vec3f::Zero();

//       switch (i) {
//       case 0:
//         e = this->dDown;
//         n_ = hit_point - ulCorner;
//         break;
//       case 1:
//         e = this->dRight;
//         n_ = hit_point - ulCorner - dDown * size;
//         break;
//       case 2:
//         e = -this->dDown;
//         n_ = hit_point - ulCorner - dDown * size - dRight * size;
//         break;
//       case 3:
//         e = -this->dRight;
//         n_ = hit_point - ulCorner - dRight * size;
//         break;
//       }
//       n = this->normal.cross(e);

//       if (n.dot(n_) < 0) {
//         return record;
//         return;
//       }
//     }

//     record.hit_point = ray.getPoint(t);
//     record.hit_normal = this->normal;
//     record.r_direction = ray.direction -
//       2 * ray.direction.dot(record.hit_normal) * record.hit_normal;

//     float wCoord = (record.hit_point - ulCorner).dot(dRight) / size * n;
//     float hCoord = (record.hit_point - ulCorner).dot(dDown) / size * n;

//     if ((int)wCoord < 0) wCoord = 0.0f;
//     if ((int)wCoord >= n - 2) wCoord = (float)(n - 2);
//     if ((int)hCoord < 0) hCoord = 0.0f;
//     if ((int)hCoord >= n - 2) hCoord = (float)(n - 2);

//     float ww1 = wCoord - floor(wCoord);
//     float ww2 = 1.0f - ww1;
//     float wh1 = hCoord - floor(hCoord);
//     float wh2 = 1.0f - wh1;

//     Vec3f color = data[(int)hCoord][(int)wCoord] * ww2 * wh2 +
//       data[(int)hCoord][(int)wCoord + 1] * ww1 * wh2 +
//       data[(int)hCoord + 1][(int)wCoord] * ww2 * wh1 +
//       data[(int)hCoord + 1][(int)wCoord + 1] * ww1 * wh1;

//     record.point_color = color;
//     record.depth = t;
//     return;
//   }

//   return record;
// }


CubeMap::CubeMap(const std::string &filepath, float size) {
  hdr_flag_ = stbi_is_hdr(filepath.c_str());
  auto stb_image = stbi_loadf(filepath.c_str(), &width_, &height_, &dimension_, 0);
  if (stb_image == nullptr) {
    throw std::runtime_error("cubemap file does not exist");
  }
  N_ = width_ > height_ ? width_ / 4 : height_ / 4;

  if (dimension_ != 3) {
    /* should be forbidden */
    exit(-1);
  }

  faces_.emplace_back(CreateFace(TOP, 0, 1,
    false, false, size, stb_image));
  faces_.emplace_back(CreateFace(BOTTOM, 2, 1,
    false, false, size, stb_image));
  faces_.emplace_back(CreateFace(LEFT, 1, 0,
    false, false, size, stb_image));
  faces_.emplace_back(CreateFace(RIGHT, 1, 2,
    false, false, size, stb_image));
  faces_.emplace_back(CreateFace(FORWARD, 1, 1,
    false, false, size, stb_image));
  if (width_ > height_) {
    faces_.emplace_back(CreateFace(BACKWARD, 1, 3,
      false, false, size, stb_image));
  } else {
    faces_.emplace_back(CreateFace(BACKWARD, 3, 1,
      true, true, size, stb_image));
  }

  stbi_image_free(stb_image);
}

std::shared_ptr<CubeMapFace> CubeMap::CreateFace(
  int face_type, int r_index, int c_index,
  bool row_inverse, bool col_inverse, float size,
  float *image_data) {

  // Calculate light's info on each face type
  Vec3f center, right_dir, down_dir;
  float half_size = size / 2.0f;
  switch (face_type) {
  case TOP:
    center = Vec3f(0.f, half_size, 0.f);
    right_dir = Vec3f(1.f, 0.f, 0.f);
    down_dir = Vec3f(0.f, 0.f, -1.f);
    break;
  case BOTTOM:
    center = Vec3f(0.f, -half_size, 0.f);
    right_dir = Vec3f(1.f, 0.f, 0.f);
    down_dir = Vec3f(0.f, 0.f, 1.f);
    break;
  case LEFT:
    center = Vec3f(-half_size, 0.f, 0.f);
    right_dir = Vec3f(0.f, 0.f, -1.f);
    down_dir = Vec3f(0.f, -1.f, 0.f);
    break;
  case RIGHT:
    center = Vec3f(half_size, 0.f, 0.f);
    right_dir = Vec3f(0.f, 0.f, 1.f);
    down_dir = Vec3f(0.f, -1.f, 0.f);
    break;
  case FORWARD:
    center = Vec3f(0.f, 0.f, -half_size);
    right_dir = Vec3f(1.f, 0.f, 0.f);
    down_dir = Vec3f(0.f, -1.f, 0.f);
    break;
  case BACKWARD:
    center = Vec3f(0.f, 0.f, half_size);
    right_dir = Vec3f(-1.f, 0.f, 0.f);
    down_dir = Vec3f(0.f, -1.f, 0.f);
    break;
  }

  std::vector<std::vector<Vec3f>> face_data(N_);
  for (auto i = 0; i < N_; ++i)
    face_data[i].resize(N_);

  int rI, imageI;
  rI = row_inverse ? r_index * N_ + N_ - 1 : r_index * N_;
  rI *= width_ * dimension_;
  for (auto r = 0; r < N_; ++r) {
    if (!col_inverse) {
      imageI = rI + c_index * N_ * dimension_;
      for (auto c = 0; c < N_; ++c) {
        face_data[r][c] = Vec3f(
          image_data[imageI], image_data[imageI + 1], image_data[imageI + 2]);
        imageI += 3;
      }
    } else {
      imageI = rI + (c_index * N_ + N_ - 1) * dimension_;
      for (auto c = 0; c < N_; ++c) {
        face_data[r][c] = Vec3f(
          image_data[imageI], image_data[imageI + 1], image_data[imageI + 2]);
        imageI -= 3;
      }
    }

    rI += row_inverse ? -width_ * dimension_ : width_ * dimension_;
  }

  return std::make_shared<CubeMapFace>(std::move(face_data),
    center, right_dir, down_dir, size);
}
void CubeMap::AppendToLightInfo(const Vec3f &s_point,
  std::vector<LightInfoToPoint> &infos) const {
  for (const auto &f : faces_) {
    f->AppendToLightInfo(s_point, infos);
  }
}
// RayHitRecord CubeMap::RayIntersection(const Ray &ray) {
//   RayHitRecord record;
//   for (int i = 0; i < 5; ++i) {
//     CubeMapFace *p;
//     if (i == 0) p = this->top;
//     else if (i == 1) p = this->left;
//     else if (i == 2) p = this->right;
//     else if (i == 3) p = this->forward;
//     else if (i == 4) p = this->backward;

//     auto tmp = p->RayIntersection(ray);
//     if (tmp.depth > MYEPSILON &&
//       (record.depth > tmp.depth || record.depth < 0.f))
//       record = tmp;
//   }
// }

}
