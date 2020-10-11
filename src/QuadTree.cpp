#include "QuadTree.h"
#include "Utils.h"

namespace ray_tracing {

QuadTree::QuadTree(const std::vector<std::vector<Vec3f>> &data, float size) {
  int n = data.size();
  pixel_size_ = size / n;

  acc_matrix_.resize(n);
  for (auto r = 0; r < n; ++r) {
    acc_matrix_[r].resize(n);
    acc_matrix_[r][0] = data[r][0].cast<double>();
    for (auto c = 1; c < n; ++c) {
      acc_matrix_[r][c] = data[r][c].cast<double>() + acc_matrix_[r][c - 1];
    }
  }
  for (auto c = 0; c < n; ++c) {
    for (auto r = 1; r < n; ++r) {
      acc_matrix_[r][c] += acc_matrix_[r - 1][c];
    }
  }

  BuildTree(0, 0, n - 1, n - 1);
}

const std::vector<NodeInfo> &QuadTree::GetNodeInfos() const {
  return node_infos_;
}

void QuadTree::BuildTree(int sr, int sc, int er, int ec) {
  Vec3f value = (acc_matrix_[er][ec] - acc_matrix_[er][sc] -
    acc_matrix_[sr][ec] + acc_matrix_[sr][sc]).cast<float>();

  if ((value.array() < COLOR_INTENSITY_THRES).all()) {
    Vec2f center((sc + ec) * pixel_size_ / 2.f,
      (sr + er) * pixel_size_ / 2.f);
    Vec2u reso(ec - sc + 1u, er - sr + 1u);

    node_infos_.emplace_back(center,
      reso.cast<float>() * pixel_size_, value);
  } else {
    int mr = (sr + er) / 2;
    int mc = (sc + ec) / 2;

    BuildTree(sr, sc, mr, mc);
    BuildTree(sr, mc + 1, mr, ec);
    BuildTree(mr + 1, sc, er, mc);
    BuildTree(mr + 1, mc + 1, er, ec);
  }
}

}
