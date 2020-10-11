#pragma once

#include <vector>
#include <memory>
#include <stack>
#include "Type.h"

namespace ray_tracing {

class Triangle;
class Ray;

class TreeNode {
  friend class Kdtree;
public:
  const AABB &GetBoundingBox() const;
  bool IsLeafNode() const;
  void EmplaceNonEmptyChilds(
    std::stack<std::shared_ptr<TreeNode>> &stack) const;
  const std::vector<uint32_t> &GetTriangleIds() const;
  bool RayHitAABB(const Ray &ray) const;

private:
  AABB aabb_;

  std::array<std::shared_ptr<TreeNode>, 2> childs_;
  std::vector<uint32_t> triangle_ids_;
};

class Kdtree {
public:
  Kdtree(std::vector<Triangle> &triangles);

  std::shared_ptr<TreeNode> GetRootNode() const;

private:
  std::shared_ptr<TreeNode> BuildTree(std::vector<Triangle> &triangles,
    uint32_t head, uint32_t tail, uint32_t level);

  std::shared_ptr<TreeNode> root_node_;
};

}
