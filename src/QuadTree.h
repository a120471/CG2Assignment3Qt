#pragma once

#include <vector>
#include "Type.h"

namespace ray_tracing {

class QuadTree {
public:
  QuadTree(Vec3f **data, int n, float size);
  ~QuadTree();

  std::vector<Vec2i> posRC;
  std::vector<Vec2f> sizeWH;
  std::vector<Vec2i> resoWH;
  std::vector<Vec3f> areaColor;

private:
  void BuildQTree(int sR, int sC, int n);

  Vec3d **sumMatrix;
  int N;
  float unitSize;
};

}
