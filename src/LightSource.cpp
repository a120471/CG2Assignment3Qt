#include "LightSource.h"
#include "GeometryObject.h"

namespace ray_tracing {

// // stb_image, Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>


bool hasHDRLighting;

PointLight::PointLight(const Vec3f &pos, const Vec3f &color)
  : pos_(pos)
  , color_(color) {
}

void PointLight::GetLight(const Vec3f &s_point,
  std::vector<Vec3f> &colors,
  std::vector<float> &distances,
  std::vector<Vec3f> &light_dirs) {
  colors.emplace_back(color_);
  distances.emplace_back((pos_, s_point).norm());
  light_dirs.emplace_back((pos_ - s_point).normalized());
}

void PointLight::RayIntersection(const Ray &ray, RayHitObjectRecord &record) {
  record.hit_point = Vec3f(0, 0, 0);
  record.hit_normal = Vec3f(0, 0, 0);
  record.r_direction = Vec3f(0, 0, 0);
  record.point_color = Vec3f(0, 0, 0);
  record.depth = -1;
}


// AreaLight::AreaLight(Vec2f areaWH, Vec2i resoWH, Vec3f pos, Vec3f totalColor, Vec3f dRight, Vec3f dDown)
//   : unitColor(totalColor / static_cast<float>(resoWH[0] * resoWH[1]))
//   , pos(pos)
//   , w(areaWH[0])
//   , h(areaWH[1]) {
//   normal = dDown.cross(dRight).normalized();

//   float numF = totalColor(0) / UNITAREASAMPLECOLOR;
//   numF = std::max(numF, totalColor(1) / UNITAREASAMPLECOLOR);
//   numF = std::max(numF, totalColor(2) / UNITAREASAMPLECOLOR);
//   int num = static_cast<int>(numF);

//   std::vector<Vec2f> point;
//   BestCandidateAlgorithm(point, num, areaWH[0], areaWH[1]);

//   for (int i = 0; i < num; ++i) {
//     Vec3f pointPos;
//     pointPos = pos + dRight * point[i](0) + dDown * point[i](1);
//     pointSamples.emplace_back(new PointLight(pointPos, unitColor / (float)num));
//   }
// }
// AreaLight::~AreaLight() {
//   for (std::vector<PointLight*>::iterator i = pointSamples.begin(); i != pointSamples.end(); ++i) {
//     SafeDelete(*i);
//   }
// }
// void AreaLight::GetLight(Vec3f sPoint, std::vector<Vec3f> &colorList, std::vector<float> &disList, std::vector<Vec3f> &lightDirList) {
//   for (int i = 0; i < pointSamples.size(); ++i) {
//     pointSamples[i]->GetLight(sPoint, colorList, disList, lightDirList);
//   }
// }
// void AreaLight::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
//   rhor.hit_point = Vec3f(0, 0, 0);
//   rhor.hit_normal = Vec3f(0, 0, 0);
//   rhor.r_direction = Vec3f(0, 0, 0);
//   rhor.point_color = Vec3f(0, 0, 0);
//   rhor.depth = -1;
// }

// SquareMap::SquareMap(Vec3f **data, int n, Vec3f ulCorner, Vec3f dRight, Vec3f dDown, float size)
//   : data(data)
//   , n(n)
//   , ulCorner(ulCorner)
//   , dRight(dRight)
//   , dDown(dDown)
//   , size(size)
//   , quadT(NULL) {
//   this->normal = dDown.cross(dRight).normalized();
//   this->D = -ulCorner.dot(normal);

//   this->quadT = new QuadTree(data, n, size);

//   for (unsigned int i = 0; i < quadT->areaColor.size(); ++i) {
//     Vec3f pos;
//     pos = ulCorner + (dRight * (float)quadT->posRC[i][1] + dDown * (float)quadT->posRC[i][0]) * (size / n);
//     this->lightSamples.emplace_back(new AreaLight(quadT->sizeWH[i], quadT->resoWH[i], pos, quadT->areaColor[i], dRight, dDown));
//   }
// }
// SquareMap::~SquareMap() {
//   for (int i = 0; i < n; ++i) {
//     delete[] data[i];
//     data[i] = NULL;
//   }
//   delete[] data;
//   data = NULL;

//   for (std::vector<AreaLight*>::iterator i = lightSamples.begin(); i != lightSamples.end(); ++i) {
//     SafeDelete(*i);
//   }
//   SafeDelete(quadT);
// }
// void SquareMap::GetLight(Vec3f sPoint, std::vector<Vec3f> &colorList, std::vector<float> &disList, std::vector<Vec3f> &lightDirList) {
//   for (unsigned int i = 0; i < this->lightSamples.size(); ++i) {
//     this->lightSamples[i]->GetLight(sPoint, colorList, disList, lightDirList);
//   }
// }
// void SquareMap::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
//   Vec3f sp = ray.sPoint;
//   Vec3f d = ray.direction;

//   float denominator = this->normal.dot(d);
//   float numerator = -this->D - this->normal.dot(sp);

//   float t = numerator / denominator;
//   while (t > MYEPSILON) {
//     Vec3f hit_point = ray.getPoint(t);
//     for (int i = 0; i < 4; ++i) {
//       Vec3f &e = Vec3f();
//       Vec3f n;
//       Vec3f &n_ = Vec3f();

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
//         rhor.hit_point = Vec3f(0, 0, 0);
//         rhor.hit_normal = Vec3f(0, 0, 0);
//         rhor.r_direction = Vec3f(0, 0, 0);
//         rhor.point_color = Vec3f(0, 0, 0);
//         rhor.depth = -1;
//         return;
//       }
//     }

//     rhor.hit_point = ray.getPoint(t);
//     rhor.hit_normal = this->normal;
//     rhor.r_direction = ray.direction - 2 * ray.direction.dot(rhor.hit_normal) * rhor.hit_normal; // it's already normalized

//     float wCoord = (rhor.hit_point - ulCorner).dot(dRight) / size * n;
//     float hCoord = (rhor.hit_point - ulCorner).dot(dDown) / size * n;

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

//     rhor.point_color = color;
//     rhor.depth = t;
//     return;
//   }

//   rhor.hit_point = Vec3f(0, 0, 0);
//   rhor.hit_normal = Vec3f(0, 0, 0);
//   rhor.r_direction = Vec3f(0, 0, 0);
//   rhor.point_color = Vec3f(0, 0, 0);
//   rhor.depth = -1;
// }

// CubeMap::CubeMap(std::string cubeMapPath, float size)
//   : loadImage(NULL) {
//   hasHDRLighting = stbi_is_hdr(cubeMapPath.c_str());
//   loadImage = stbi_loadf(cubeMapPath.c_str(), &width, &height, &dimension, 0);
//   N = width > height ? width / 4 : height / 4;

//   if (dimension != 3) {
//     /* should be forbidden */
//     exit(-1);
//   }

//   ExtractSquareMap(top, 0, 0, 1, false, false, size);    // top
//   ExtractSquareMap(bottom, 1, 2, 1, false, false, size);  // bottom
//   ExtractSquareMap(left, 2, 1, 0, false, false, size);  // left
//   ExtractSquareMap(right, 3, 1, 2, false, false, size);  // right
//   ExtractSquareMap(forward, 4, 1, 1, false, false, size);  // forward
//   if (width > height)
//     ExtractSquareMap(backward, 5, 1, 3, false, false, size);// backward
//   else
//     ExtractSquareMap(backward, 5, 3, 1, true, true, size);  // backward

//   stbi_image_free(loadImage);
// }
// CubeMap::CubeMap(std::string cubeMapPath[]) {
//   /* to do */
// }
// CubeMap::~CubeMap() {
//   SafeDelete(top);
//   SafeDelete(bottom);
//   SafeDelete(left);
//   SafeDelete(right);
//   SafeDelete(forward);
//   SafeDelete(backward);
// }
// void CubeMap::ExtractSquareMap(SquareMap *&sm, int idx, int rowIdx, int colIdx, bool rowInverse, bool colInverse, float size) {
//   Vec3f **area = new Vec3f*[N];
//   for (int i = 0; i < N; ++i)
//     area[i] = new Vec3f[N];
//   // do not delete area, we delete it in ~SquareMap()

//   int rI, imageI;
//   rI = rowInverse ? rowIdx * N + N - 1 : rowIdx * N;
//   rI *= width * dimension;
//   for (int r = 0; r < N; ++r) {
//     if (!colInverse) {
//       imageI = rI + colIdx * N * dimension;
//       for (int c = 0; c < N; ++c) {
//         area[r][c] = Vec3f(loadImage[imageI], loadImage[imageI + 1], loadImage[imageI + 2]);
//         imageI += 3;
//       }
//     }
//     else {
//       imageI = rI + (colIdx + 1) * N * dimension - 1;
//       for (int c = 0; c < N; ++c) {
//         area[r][c] = Vec3f(loadImage[imageI - 2], loadImage[imageI - 1], loadImage[imageI]);
//         imageI -= 3;
//       }
//     }

//     rI += rowInverse ? -width * dimension : width * dimension;
//   }

//   // to calculate area light's pos
//   Vec3f ulCorner, dR, dD;
//   float size_2 = size / 2.0f;
//   switch (idx) {
//   case 0: // top
//     ulCorner = Vec3f(-size_2, size_2, size_2);
//     dR = Vec3f(1, 0, 0);
//     dD = Vec3f(0, 0, -1);
//     break;
//   case 1: // bottom
//     ulCorner = Vec3f(-size_2, -size_2, -size_2);
//     dR = Vec3f(1, 0, 0);
//     dD = Vec3f(0, 0, 1);
//     break;
//   case 2: // left
//     ulCorner = Vec3f(-size_2, size_2, size_2);
//     dR = Vec3f(0, 0, -1);
//     dD = Vec3f(0, -1, 0);
//     break;
//   case 3: // right
//     ulCorner = Vec3f(size_2, size_2, -size_2);
//     dR = Vec3f(0, 0, 1);
//     dD = Vec3f(0, -1, 0);
//     break;
//   case 4: // forward
//     ulCorner = Vec3f(-size_2, size_2, -size_2);
//     dR = Vec3f(1, 0, 0);
//     dD = Vec3f(0, -1, 0);
//     break;
//   case 5: // backward
//     ulCorner = Vec3f(size_2, size_2, size_2);
//     dR = Vec3f(-1, 0, 0);
//     dD = Vec3f(0, -1, 0);
//     break;
//   }

//   sm = new SquareMap(area, N, ulCorner, dR, dD, size);
// }
// void CubeMap::GetLight(Vec3f sPoint, std::vector<Vec3f> &colorList, std::vector<float> &disList, std::vector<Vec3f> &lightDirList) {
//   this->top->GetLight(sPoint, colorList, disList, lightDirList);
//   // this->bottom->GetLight(sPoint, colorList, disList, lightDirList);
//   this->left->GetLight(sPoint, colorList, disList, lightDirList);
//   this->right->GetLight(sPoint, colorList, disList, lightDirList);
//   this->forward->GetLight(sPoint, colorList, disList, lightDirList);
//   this->backward->GetLight(sPoint, colorList, disList, lightDirList);
// }
// void CubeMap::RayIntersection(const Ray &ray, RayHitObjectRecord &rhor) {
//   RayHitObjectRecord rhorT;
//   for (int i = 0; i < 5; ++i) {
//     SquareMap *p;
//     if (i == 0) p = this->top;
//     else if (i == 1) p = this->left;
//     else if (i == 2) p = this->right;
//     else if (i == 3) p = this->forward;
//     else if (i == 4) p = this->backward;

//     p->RayIntersection(ray, rhorT);
//     if (rhorT.depth > MYEPSILON && (rhor.depth > rhorT.depth || rhor.depth < MYEPSILON))
//       rhor = rhorT;
//   }
// }

}
