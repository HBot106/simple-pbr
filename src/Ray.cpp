#include "Ray.hpp"

Ray::Ray(glm::vec3 ray_origin, glm::vec3 ray_direction)
{
	type = "ray";
	origin = ray_origin;
	direction = ray_direction;
	boundingBox.resetToPoint(origin);
}

Ray::Ray(const Ray &obj)
{
	type = obj.type;
	origin = obj.origin;
	direction = obj.direction;
	boundingBox.resetToPoint(origin);
}

void Ray::PrintAttributes()
{
	std::cout << "==============================================================================\n";
	std::cout << "Printing Ray Attributes:\n";
	std::cout << "origin:\t\t\t" << origin.x << ",\t\t" << origin.y << ",\t\t" << origin.z << "\n";
	std::cout << "direction:\t\t" << direction.x << ",\t" << direction.y << ",\t" << direction.z << "\n";
	std::cout << "\n";
}

aaBoundingBox Ray::getAABB()
{
	return boundingBox;
}
glm::mat4 Ray::getModelMatrix()
{
  return glm::mat4(1.0f);
}
glm::mat4 Ray::getModelInverseMatrix()
{
  return glm::mat4(1.0f);
}
glm::mat4 Ray::getModelInverseTransposeMatrix()
{
  return glm::mat4(1.0f);
}
std::string Ray::getType()
{
	return type;
}
glm::vec3 Ray::getPigment()
{
	return glm::vec3(0.0f, 0.0f, 0.0f);
}
glm::vec3 Ray::getLocation()
{
	return origin;
}
float Ray::getAmbient()
{
	return 0.0f;
}
float Ray::getSpecular()
{
	return 0.0f;
}
float Ray::getDiffuse()
{
	return 0.0f;
}
float Ray::getReflection()
{
	return 0.0f;
}
float Ray::getRefraction()
{
	return 0.0f;
}
float Ray::getRoughness()
{
	return 0.0f;
}
glm::vec3 Ray::getNormal(glm::vec3 intersectionPoint)
{
	return glm::vec3(0.0f, 0.0f, 0.0f);
}
float  Ray::getIor()
{
	return 0.0f;
}
float Ray::getFilter()
{
	return 0.0f;
}
void Ray::updatePhysics(float frametime)
{
	return;
}

float Ray::ComputeIntersection(glm::vec3 origin, glm::vec3 direction)
{
	return std::numeric_limits<float>::max();
}

glm::vec3 Ray::compute_diffuse(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::vec3 Ray::compute_specular(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::vec3 Ray::compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}