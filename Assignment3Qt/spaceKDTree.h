#pragma once

#include <vector>

#include <glm/gtc/type_ptr.hpp>

class Triangle; // include "geometryObject.h"

class SpaceKDTree
{
public:
	struct TreeNode
	{
		TreeNode()
		{
			lChild = NULL;
			rChild = NULL;
			triangleIdx.clear();
		}

		// bounding box
		glm::vec3 AA; // min corner
		glm::vec3 BB; // max corner

		TreeNode *lChild, *rChild;
		std::vector<int> triangleIdx;
	};

	SpaceKDTree(std::vector<Triangle*> &faces);
	~SpaceKDTree();

	TreeNode* rootNode; // don't forget to set it to NULL

private:
	void BuildKDTree(std::vector<Triangle*> &faces, int head, int tail, int level, TreeNode *&node);
	void DeleteKDTree(TreeNode *&node);
};