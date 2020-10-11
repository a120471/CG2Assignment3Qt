#pragma once

#include <vector>
#include "Type.h"

namespace ray_tracing {

struct NodeInfo {
  NodeInfo(const Vec2f &p, const Vec2f &s, const Vec3f &c)
  : pos(p)
  , size(s)
  , sum_color(c) {
  }

  Vec2f pos;
  Vec2f size;
  Vec3f sum_color;
};

class QuadTree {
public:
  QuadTree(const std::vector<std::vector<Vec3f>> &data, float size);

  const std::vector<NodeInfo> &GetNodeInfos() const;

private:
  void BuildTree(int sr, int sc, int er, int ec);

  std::vector<std::vector<Vec3d>> acc_matrix_;
  float pixel_size_;

  std::vector<NodeInfo> node_infos_;
};

}
