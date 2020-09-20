#pragma once

#include <vector>
#include <memory>
#include "Type.h"

namespace ray_tracing {

class Triangle;

class KDTree {
public:
  struct TreeNode {
    // bounding box
    Vec3f AA;
    Vec3f BB;

    TreeNode *l_child, *r_child;
    std::vector<int> face_ids;
  };

  KDTree(const std::vector<std::shared_ptr<Triangle>> &faces);
  ~KDTree();

  TreeNode *GetRootNode();

private:
  void BuildKDTree(int head, int tail, int level, TreeNode *&node);
  void DeleteKDTree(TreeNode *&node);

  TreeNode *root_node_{nullptr};
  std::vector<std::shared_ptr<Triangle>> faces_;
};

}
