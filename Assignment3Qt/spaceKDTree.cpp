#include "spaceKDTree.h"

#include "geometryObject.h"

SpaceKDTree::SpaceKDTree(std::vector<Triangle*> &faces)
	: rootNode(NULL)
{
	BuildKDTree(faces, 0, faces.size(), 0, rootNode);
}

SpaceKDTree::~SpaceKDTree()
{
	DeleteKDTree(this->rootNode);
}

void SpaceKDTree::BuildKDTree(std::vector<Triangle*> &faces, int head, int tail, int level, TreeNode *&node)
{
	node = new TreeNode();

	if (tail - head <= 4)
	{
		for (int i = head; i < tail; i++)
			node->triangleIdx.push_back(i);
		// compute the bounding box
		faces[head]->GetBoundingBox(node->AA, node->BB);
		for (std::vector<Triangle*>::iterator i = faces.begin() + head + 1; i < faces.begin() + tail; i++)
		{
			glm::vec3 AT, BT;
			(*i)->GetBoundingBox(AT, BT);
			MergeBoundingBox(node->AA, node->BB, node->AA, node->BB, AT, BT);
		}

		return;
	}

	switch (level % 3)
	{
	case 0:
		sort(faces.begin() + head, faces.begin() + tail, Mesh::SortByX);
		break;
	case 1:
		sort(faces.begin() + head, faces.begin() + tail, Mesh::SortByY);
		break;
	case 2:
		sort(faces.begin() + head, faces.begin() + tail, Mesh::SortByZ);
		break;
	}

	int middle = (head + tail) / 2;
	BuildKDTree(faces, head, middle, level + 1, node->lChild);
	BuildKDTree(faces, middle, tail, level + 1, node->rChild);

	MergeBoundingBox(node->AA, node->BB, node->lChild->AA, node->lChild->BB, node->rChild->AA, node->rChild->BB);
}

void SpaceKDTree::DeleteKDTree(TreeNode *&node)
{
	if (node)
	{
		DeleteKDTree(node->lChild);
		DeleteKDTree(node->rChild);
		safe_delete(node);
	}
}