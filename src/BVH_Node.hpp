#ifndef BVH_Node_H
#define BVH_Node_H

#include <vector>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../src/SceneObject.hpp"

class BVH_Node
{
public:

	BVH_Node * parent;
	BVH_Node * child1;
	BVH_Node * child2;
	aaBoundingBox currentAABB;
	std::vector<SceneObject&> nodeObjectList;

	void buildTree(std::vector<SceneObject&> sceneObjectList, int axis)
	{
		if (sceneObjectList.size() < 10)
		{
			nodeObjectList = sceneObjectList;

			currentAABB.resetToPoint(sceneObjectList[0].boundingBox.min);
			for (int i = 0; i < sceneObjectList.size(); i++)
			{
				currentAABB.addBox(sceneObjectList[i].getAABB());
			}
			return;
		}
		else
		{
			sortObjects(sceneObjectList, axis);

			std::vector<SceneObject&> sceneObjectSubList1, sceneObjectSubList2;
			for (int i = 0; i < (sceneObjectList.size() / 2); i++)
			{
				sceneObjectSubList1.push_back[i];
			}
			for (int i = (sceneObjectList.size() / 2); i < sceneObjectList.size(); i++)
			{
				sceneObjectSubList1.push_back[i];
			}

			child1 = new BVH_Node();
			child2 = new BVH_Node();

			child1->buildTree(sceneObjectSubList1, (axis + 1) % 3);
			child2->buildTree(sceneObjectSubList2, (axis + 1) % 3);

			currentAABB.addBox(child1->currentAABB);
			currentAABB.addBox(child2->currentAABB);
			return;
		}
	}

	void sortObjects(std::vector<SceneObject&> & sceneObjectList, int axis)
	{
		if (axis == 0)
		{
			std::sort(sceneObjectList.begin(), sceneObjectList.end(), sortByXPosition);
		}
		else if (axis == 1)
		{
			std::sort(sceneObjectList.begin(), sceneObjectList.end(), sortByYPosition);
		}
		else if (axis == 2)
		{
			std::sort(sceneObjectList.begin(), sceneObjectList.end(), sortByZPosition);
		}
		else
		{
			std::sort(sceneObjectList.begin(), sceneObjectList.end(), sortByXPosition);
		}
	}

	bool sortByXPosition(const SceneObject & object1, const SceneObject & object2)
	{
		return (object1.boundingBox.min.x < object2.boundingBox.min.x);
	}

	bool sortByYPosition(const SceneObject & object1, const SceneObject & object2)
	{
		return (object1.boundingBox.min.y < object2.boundingBox.min.y);
	}

	bool sortByZPosition(const SceneObject & object1, const SceneObject & object2)
	{
		return (object1.boundingBox.min.z < object2.boundingBox.min.z);
	}
};
#endif