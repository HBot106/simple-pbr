#include "Camera.hpp"

Camera::Camera(glm::vec3 camera_location, glm::vec3 camera_up, glm::vec3 camera_right, glm::vec3 camera_look_at)
{
	type = "camera";
	location = camera_location;
	up = camera_up;
	right = camera_right;
	look_at = camera_look_at;
	boundingBox.resetToPoint(location);
}

Camera::Camera(const Camera &obj)
{
	type = obj.type;
	location = obj.location;
	up = obj.up;
	right = obj.right;
	look_at = obj.look_at;
	boundingBox.resetToPoint(location);
}


void Camera::PrintAttributes()
{
	std::cout << "==============================================================================\n";
	std::cout << "Printing Camera Attributes:\n";
	std::cout << "location:\t" << location.x << ",\t" << location.y << ",\t" << location.z << "\n";
	std::cout << "up:\t\t" << up.x << ",\t" << up.y << ",\t" << up.z << "\n";
	std::cout << "right:\t\t" << right.x << ",\t" << right.y << ",\t" << right.z << "\n";
	std::cout << "look_at:\t" << look_at.x << ",\t" << look_at.y << ",\t" << look_at.z << "\n";
	std::cout << "\n";
}

aaBoundingBox Camera::getAABB()
{
	return boundingBox;
}
glm::mat4 Camera::getModelMatrix()
{
  return glm::mat4(1.0f);
}
glm::mat4 Camera::getModelInverseMatrix()
{
  return glm::mat4(1.0f);
}
glm::mat4 Camera::getModelInverseTransposeMatrix()
{
  return glm::mat4(1.0f);
}
std::string Camera::getType()
{
	return type;
}
glm::vec3 Camera::getPigment()
{
	return glm::vec3(0.0f, 0.0f, 0.0f);
}
glm::vec3 Camera::getLocation()
{
	return location;
}
float Camera::getAmbient()
{
	return 0.0f;
}
float Camera::getSpecular()
{
	return 0.0f;
}
float Camera::getDiffuse()
{
	return 0.0f;
}
float Camera::getReflection()
{
	return 0.0f;
}
float Camera::getRefraction()
{
	return 0.0f;
}
float Camera::getRoughness()
{
	return 0.0f;
}
glm::vec3 Camera::getNormal(glm::vec3 intersectionPoint)
{
	return glm::vec3(0.0f, 0.0f, 0.0f);
}
float Camera::getIor()
{
	return 0.0f;
}
float Camera::getFilter()
{
	return 0.0f;
}
void Camera::updatePhysics(float frametime)
{
	return;
}

float Camera::ComputeIntersection(glm::vec3 origin, glm::vec3 direction)
{
	return std::numeric_limits<float>::max();
}

glm::vec3 Camera::compute_diffuse(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::vec3 Camera::compute_specular(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::vec3 Camera::compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}