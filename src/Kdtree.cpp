#include "Kdtree.h"
#include "GeometryObject.h"
#include "RayTracingCamera.h"

namespace {

using namespace ray_tracing;

const float EPSILON = 1e-4f; // todo

inline bool ApproxiSmaller(float a, float b) {
  return a < b + EPSILON;
}

bool SortByX(const Triangle &t1, const Triangle &t2) {
  return t1.GetBaryCenter()(0) < t2.GetBaryCenter()(0);
}
bool SortByY(const Triangle &t1, const Triangle &t2) {
  return t1.GetBaryCenter()(1) < t2.GetBaryCenter()(1);
}
bool SortByZ(const Triangle &t1, const Triangle &t2) {
  return t1.GetBaryCenter()(2) < t2.GetBaryCenter()(2);
}

}

namespace ray_tracing {

const AABB &TreeNode::GetBoundingBox() const {
  return aabb_;
}

bool TreeNode::IsLeafNode() const {
  return !triangle_ids_.empty();
}

void TreeNode::EmplaceNonEmptyChilds(
  std::stack<std::shared_ptr<TreeNode>> &stack) const {
  for (auto &child : childs_) {
    if (child) {
      stack.emplace(child);
    }
  }
}

const std::vector<uint32_t> &TreeNode::GetTriangleIds() const {
  return triangle_ids_;
}

// Reference: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool TreeNode::RayHitAABB(const Ray &ray) const {
  const Vec3f &ray_o = ray.s_point;
  const Vec3f &normalized_d = ray.direction;

  const Vec3f &min_corner = aabb_.min();
  const Vec3f &max_corner = aabb_.max();
  Vec3f min_dc = min_corner - ray_o;
  Vec3f max_dc = max_corner - ray_o;
  float max_tmin = -std::numeric_limits<float>::max();
  float min_tmax = std::numeric_limits<float>::max();

  for (int i = 0; i < 3; ++i) {
    if (abs(normalized_d(i)) > EPSILON) {
      float t1 = min_dc(i) / normalized_d(i);
      float t2 = max_dc(i) / normalized_d(i);
      if (t1 > t2) {
        std::swap(t1, t2);
      }
      max_tmin = std::max(max_tmin, t1);
      min_tmax = std::min(min_tmax, t2);
    } else if (min_dc(i) * max_dc(i) > EPSILON * EPSILON) {
      return false;
    }
  }

  if (ApproxiSmaller(max_tmin, min_tmax) && min_tmax > 0) {
    return true;
  } else {
    return false;
  }
}


Kdtree::Kdtree(std::vector<Triangle> &triangles) {
  root_node_ = BuildTree(triangles, 0u, triangles.size(), 0u);
}

std::shared_ptr<TreeNode> Kdtree::GetRootNode() const {
  return root_node_;
}

std::shared_ptr<TreeNode> Kdtree::BuildTree(std::vector<Triangle> &triangles,
  uint32_t head, uint32_t tail, uint32_t level) {
  auto node = std::make_shared<TreeNode>();

  if (tail - head <= 4u) {
    for (auto i = head; i < tail; ++i) {
      node->triangle_ids_.emplace_back(i);
    }
    // Compute the bounding box
    for (auto i = head; i < tail; ++i) {
      node->aabb_.extend(triangles[i].GetBoundingBox());
    }
  } else {
    switch (level % 3u) {
    case 0:
      std::sort(triangles.begin() + head, triangles.begin() + tail, SortByX);
      break;
    case 1:
      std::sort(triangles.begin() + head, triangles.begin() + tail, SortByY);
      break;
    case 2:
      std::sort(triangles.begin() + head, triangles.begin() + tail, SortByZ);
      break;
    }

    uint32_t middle = (head + tail) / 2u;
    node->childs_[0] = BuildTree(triangles, head, middle, level + 1);
    node->childs_[1] = BuildTree(triangles, middle, tail, level + 1);

    node->aabb_.extend(node->childs_[0]->aabb_).extend(node->childs_[1]->aabb_);
  }

  return node;
}

}
