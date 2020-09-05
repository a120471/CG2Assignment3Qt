#include "QuadTree.h"
#include "Utils.h"

namespace ray_tracing {

QuadTree::QuadTree(glm::vec3 **data, int n, float size)
  : sumMatrix(NULL) {
  this->sumMatrix = new glm::dvec3*[n];
  for (int i = 0; i < n; ++i) {
    this->sumMatrix[i] = new glm::dvec3[n];
    memset(this->sumMatrix[i], 0, sizeof(float) * n);
  }
  this->N = n;
  this->unitSize = size / n;

  sumMatrix[0][0] = data[0][0];
  for (int c = 1; c < n; ++c) {
    sumMatrix[0][c] = sumMatrix[0][c - 1] + glm::dvec3(data[0][c].x, data[0][c].y, data[0][c].z);
  }
  for (int r = 1; r < n; ++r) {
    sumMatrix[r][0] = sumMatrix[r - 1][0] + glm::dvec3(data[r][0].x, data[r][0].y, data[r][0].z);
  }
  for (int r = 1; r < n; ++r) {
    for (int c = 1; c < n; ++c) {
      sumMatrix[r][c] = sumMatrix[r][c - 1] + sumMatrix[r - 1][c] - sumMatrix[r - 1][c - 1] +
        glm::dvec3(data[r][c].x, data[r][c].y, data[r][c].z);
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
  glm::vec3 value = sumMatrix[sR + n - 1][sC + n - 1] + sumMatrix[sR][sC] - sumMatrix[sR + n - 1][sC] - sumMatrix[sR][sC + n - 1];
  if (value.x < COLORINTENSITYTHRES && value.y < COLORINTENSITYTHRES && value.z < COLORINTENSITYTHRES) {
    posRC.emplace_back(sR + n_2, sC + n_2);
    sizeWH.emplace_back(n * unitSize);
    resoWH.emplace_back(n);
    areaColor.emplace_back(value);
    return;
  }

  BuildQTree(sR, sC, n_2);
  BuildQTree(sR, sC + n_2, n_2);
  BuildQTree(sR + n_2, sC, n_2);
  BuildQTree(sR + n_2, sC + n_2, n_2);
}

}
