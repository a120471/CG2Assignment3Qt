#include "KDTree.h"
#include <algorithm>
#include "GeometryObject.h"
#include "Utils.h"

namespace {

using namespace ray_tracing;

bool SortByX(const std::shared_ptr<Triangle> &t1,
  const std::shared_ptr<Triangle> &t2) {
  return t1->GetBaryCenter()(0) < t2->GetBaryCenter()(0);
}
bool SortByY(const std::shared_ptr<Triangle> &t1,
  const std::shared_ptr<Triangle> &t2) {
  return t1->GetBaryCenter()(1) < t2->GetBaryCenter()(1);
}
bool SortByZ(const std::shared_ptr<Triangle> &t1,
  const std::shared_ptr<Triangle> &t2) {
  return t1->GetBaryCenter()(2) < t2->GetBaryCenter()(2);
}

}

namespace ray_tracing {

KDTree::KDTree(const std::vector<std::shared_ptr<Triangle>> &faces)
  : faces_(faces) {
  BuildKDTree(0, faces_.size(), 0, root_node_);
}

KDTree::~KDTree() {
  DeleteKDTree(root_node_);
}

KDTree::TreeNode *KDTree::GetRootNode() {
  return root_node_;
}

void KDTree::BuildKDTree(int head, int tail, int level, TreeNode *&node) {
  node = new TreeNode();

  if (tail - head <= 4) {
    for (int i = head; i < tail; ++i) {
      node->face_ids.emplace_back(i);
    }
    // compute the bounding box
    faces_[head]->GetBoundingBox(node->AA, node->BB);
    for (auto i = faces_.begin() + head + 1; i < faces_.begin() + tail; ++i) {
      Vec3f AT, BT;
      (*i)->GetBoundingBox(AT, BT);
      MergeBoundingBox(node->AA, node->BB, node->AA, node->BB, AT, BT);
    }

    return;
  }

  switch (level % 3) {
  case 0:
    std::sort(faces_.begin() + head, faces_.begin() + tail, SortByX);
    break;
  case 1:
    std::sort(faces_.begin() + head, faces_.begin() + tail, SortByY);
    break;
  case 2:
    std::sort(faces_.begin() + head, faces_.begin() + tail, SortByZ);
    break;
  }

  int middle = (head + tail) / 2;
  BuildKDTree(head, middle, level + 1, node->l_child);
  BuildKDTree(middle, tail, level + 1, node->r_child);

  MergeBoundingBox(node->AA, node->BB,
    node->l_child->AA, node->l_child->BB,
    node->r_child->AA, node->r_child->BB);
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
