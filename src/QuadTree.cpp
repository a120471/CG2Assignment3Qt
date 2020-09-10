#include "QuadTree.h"
#include "Utils.h"

namespace ray_tracing {

QuadTree::QuadTree(Vec3f **data, int n, float size)
  : sumMatrix(NULL) {
  this->sumMatrix = new Vec3d*[n];
  for (int i = 0; i < n; ++i) {
    this->sumMatrix[i] = new Vec3d[n];
    memset(this->sumMatrix[i], 0, sizeof(float) * n);
  }
  this->N = n;
  this->unitSize = size / n;

  sumMatrix[0][0] = data[0][0].cast<double>();
  for (int c = 1; c < n; ++c) {
    sumMatrix[0][c] = sumMatrix[0][c - 1] + Vec3d(data[0][c](0), data[0][c](1), data[0][c](2));
  }
  for (int r = 1; r < n; ++r) {
    sumMatrix[r][0] = sumMatrix[r - 1][0] + Vec3d(data[r][0](0), data[r][0](1), data[r][0](2));
  }
  for (int r = 1; r < n; ++r) {
    for (int c = 1; c < n; ++c) {
      sumMatrix[r][c] = sumMatrix[r][c - 1] + sumMatrix[r - 1][c] - sumMatrix[r - 1][c - 1] +
        Vec3d(data[r][c](0), data[r][c](1), data[r][c](2));
    }
  }

  BuildQTree(0, 0, n);
}

QuadTree::~QuadTree() {
  for (int i = 0; i < N; ++i) {
    delete[] sumMatrix[i];
  }
  delete[] sumMatrix;
}

void QuadTree::BuildQTree(int sR, int sC, int n) {
  int n_2 = n / 2;
  Vec3f value = (sumMatrix[sR + n - 1][sC + n - 1] + sumMatrix[sR][sC] - sumMatrix[sR + n - 1][sC] - sumMatrix[sR][sC + n - 1]).cast<float>();
  if (value(0) < COLORINTENSITYTHRES && value(1) < COLORINTENSITYTHRES && value(2) < COLORINTENSITYTHRES) {
    posRC.emplace_back(sR + n_2, sC + n_2);
    sizeWH.emplace_back(Vec2f::Ones() * n * unitSize);
    resoWH.emplace_back(Vec2i::Ones() * n);
    areaColor.emplace_back(value);
    return;
  }

  BuildQTree(sR, sC, n_2);
  BuildQTree(sR, sC + n_2, n_2);
  BuildQTree(sR + n_2, sC, n_2);
  BuildQTree(sR + n_2, sC + n_2, n_2);
}

}
