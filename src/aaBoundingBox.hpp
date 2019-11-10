#ifndef AABB_H
#define AABB_H

#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../src/SceneObject.hpp"

class aaBoundingBox
{
public:

	glm::vec3 min, max;

	void resetToPoint(glm::vec3 point)
	{
		min = max = point;
	}

	void addPoint(glm::vec3 point)
	{
		min.x = glm::min(min.x, point.x);
		min.y = glm::min(min.y, point.y);
		min.z = glm::min(min.z, point.z);

		max.x = glm::max(max.x, point.x);
		max.y = glm::max(max.y, point.y);
		max.z = glm::max(max.z, point.z);
	}

	void addBox(aaBoundingBox subBox)
	{
		addPoint(subBox.min);
		addPoint(subBox.max);
	}

	void transformAABB(glm::mat4 modelMatrix)
	{
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> verticesTransformed;

		vertices.push_back(glm::vec3(min.x, min.y, min.z));
		vertices.push_back(glm::vec3(min.x, min.y, max.z));
		vertices.push_back(glm::vec3(min.x, max.y, min.z));
		vertices.push_back(glm::vec3(min.x, max.y, max.z));

		vertices.push_back(glm::vec3(max.x, min.y, min.z));
		vertices.push_back(glm::vec3(max.x, min.y, max.z));
		vertices.push_back(glm::vec3(max.x, max.y, min.z));
		vertices.push_back(glm::vec3(max.x, max.y, max.z));

		for (glm::vec3 & vertex : vertices)
		{
			glm::vec4 transformVertex = modelMatrix * glm::vec4(vertex, 1.0f);
			verticesTransformed.push_back(glm::vec3(transformVertex));
		}

		resetToPoint(verticesTransformed[0]);
		for (int i = 1; i < 8; i++)
		{
			addPoint(verticesTransformed[i]);
		}
	}
};
#endif