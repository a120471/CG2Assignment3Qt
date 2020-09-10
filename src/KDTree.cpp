#include "KDTree.h"
#include <algorithm>
#include "GeometryObject.h"
#include "Utils.h"

namespace ray_tracing {

KDTree::KDTree(std::vector<Triangle*> &faces)
  : rootNode(nullptr) {
  BuildKDTree(faces, 0, faces.size(), 0, rootNode);
}

KDTree::~KDTree() {
  DeleteKDTree(rootNode);
}

void KDTree::BuildKDTree(std::vector<Triangle*> &faces,
  int head, int tail, int level, TreeNode *&node) {
  node = new TreeNode();

  if (tail - head <= 4) {
    for (int i = head; i < tail; ++i) {
      node->triangleIdx.emplace_back(i);
    }
    // compute the bounding box
    faces[head]->GetBoundingBox(node->AA, node->BB);
    for (auto i = faces.begin() + head + 1; i < faces.begin() + tail; ++i) {
      Vec3f AT, BT;
      (*i)->GetBoundingBox(AT, BT);
      MergeBoundingBox(node->AA, node->BB, node->AA, node->BB, AT, BT);
    }

    return;
  }

  switch (level % 3) {
  case 0:
    std::sort(faces.begin() + head, faces.begin() + tail, Mesh::SortByX);
    break;
  case 1:
    std::sort(faces.begin() + head, faces.begin() + tail, Mesh::SortByY);
    break;
  case 2:
    std::sort(faces.begin() + head, faces.begin() + tail, Mesh::SortByZ);
    break;
  }

  int middle = (head + tail) / 2;
  BuildKDTree(faces, head, middle, level + 1, node->lChild);
  BuildKDTree(faces, middle, tail, level + 1, node->rChild);

  MergeBoundingBox(node->AA, node->BB,
    node->lChild->AA, node->lChild->BB, node->rChild->AA, node->rChild->BB);
}

void KDTree::DeleteKDTree(TreeNode *&node) {
  if (node) {
    DeleteKDTree(node->lChild);
    DeleteKDTree(node->rChild);
    SafeDelete(node);
  }
}

}
