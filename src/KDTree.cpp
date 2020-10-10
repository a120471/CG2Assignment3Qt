#include "KDTree.h"
#include <algorithm>
#include "GeometryObject.h"
#include "Utils.h"

namespace {

using namespace ray_tracing;

bool SortByX(const Triangle &t1,
  const Triangle &t2) {
  return t1.GetBaryCenter()(0) < t2.GetBaryCenter()(0);
}
bool SortByY(const Triangle &t1,
  const Triangle &t2) {
  return t1.GetBaryCenter()(1) < t2.GetBaryCenter()(1);
}
bool SortByZ(const Triangle &t1,
  const Triangle &t2) {
  return t1.GetBaryCenter()(2) < t2.GetBaryCenter()(2);
}

}

namespace ray_tracing {

KDTree::KDTree(const std::vector<std::shared_ptr<Triangle>> &faces) {
  BuildKDTree(0, faces.size(), 0, root_node_);
}

KDTree::~KDTree() {
  DeleteKDTree(root_node_);
}

KDTree::TreeNode *KDTree::GetRootNode() {
  return root_node_;
}

void KDTree::BuildKDTree(std::vector<Triangle> &faces,
  int head, int tail, int level, TreeNode *&node) {
  node = new TreeNode();

  if (tail - head <= 4) {
    for (int i = head; i < tail; ++i) {
      node->face_ids.emplace_back(i);
    }
    // compute the bounding box
    for (auto i = head; i < tail; ++i) {
      node->aabb.extend(faces[i].GetBoundingBox());
    }
  } else {
    switch (level % 3) {
    case 0:
      std::sort(faces.begin() + head, faces.begin() + tail, SortByX);
      break;
    case 1:
      std::sort(faces.begin() + head, faces.begin() + tail, SortByY);
      break;
    case 2:
      std::sort(faces.begin() + head, faces.begin() + tail, SortByZ);
      break;
    }

    int middle = (head + tail) / 2;
    BuildKDTree(head, middle, level + 1, node->l_child);
    BuildKDTree(middle, tail, level + 1, node->r_child);

    node->aabb.extend(node->l_child->aabb).extend(
      node->r_child->aabb);
  }
}

void KDTree::DeleteKDTree(TreeNode *&node) {
  if (node) {
    DeleteKDTree(node->l_child);
    DeleteKDTree(node->r_child);
    delete(node);
    node = nullptr;
  }
}

}
