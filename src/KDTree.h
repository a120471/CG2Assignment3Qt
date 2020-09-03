#pragma once

#include <vector>
#include <glm/gtc/type_ptr.hpp>

namespace ray_tracing {

class Triangle;

class KDTree {
public:
  struct TreeNode {
    TreeNode() {
      lChild = nullptr;
      rChild = nullptr;
      triangleIdx.clear();
    }

    // bounding box
    glm::vec3 AA; // min corner
    glm::vec3 BB; // max corner

    TreeNode *lChild, *rChild;
    std::vector<int> triangleIdx;
  };

  KDTree(std::vector<Triangle*> &faces);
  ~KDTree();

  TreeNode* rootNode; // don't forget to set it to nullptr

private:
  void BuildKDTree(std::vector<Triangle*> &faces,
    int head, int tail, int level, TreeNode *&node);
  void DeleteKDTree(TreeNode *&node);
};

}
