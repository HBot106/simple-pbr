#include "LightSource.hpp"

LightSource::LightSource(glm::vec3 light_source_location, glm::vec3 light_source_color)
{
	type = "light";
	location = light_source_location;
	color = light_source_color;
	boundingBox.resetToPoint(location);
}

LightSource::LightSource(const LightSource &obj)
{
	type = obj.type;
	location = obj.location;
	color = obj.color;
	boundingBox.resetToPoint(location);
}

void LightSource::PrintAttributes()
{
	std::cout << "==============================================================================\n";
	std::cout << "Printing Light Source Attributes:\n";
	std::cout << "location:\t" << location.x << ",\t" << location.y << ",\t" << location.z << "\n";
	std::cout << "location:\t" << color.x << ",\t" << color.y << ",\t" << color.z << "\n";
	std::cout << "\n";
}

aaBoundingBox LightSource::getAABB()
{
	return boundingBox;
}
glm::mat4 LightSource::getModelMatrix()
{
  return glm::mat4(1.0f);
}
glm::mat4 LightSource::getModelInverseMatrix()
{
  return glm::mat4(1.0f);
}
glm::mat4 LightSource::getModelInverseTransposeMatrix()
{
  return glm::mat4(1.0f);
}
std::string LightSource::getType()
{
	return type;
}
glm::vec3 LightSource::getPigment()
{
	return glm::vec3(0.0f, 0.0f, 0.0f);
}
glm::vec3 LightSource::getLocation()
{
	return location;
}
float LightSource::getAmbient()
{
	return 0.0f;
}
float LightSource::getSpecular()
{
	return 0.0f;
}
float LightSource::getDiffuse()
{
	return 0.0f;
}
float LightSource::getReflection()
{
	return 0.0f;
}
float LightSource::getRefraction()
{
	return 0.0f;
}
float LightSource::getRoughness()
{
	return 0.0f;
}
glm::vec3 LightSource::getNormal(glm::vec3 intersectionPoint)
{
	return glm::vec3(0.0f, 0.0f, 0.0f);
}
float  LightSource::getIor()
{
	return 0.0f;
}
float LightSource::getFilter()
{
	return 0.0f;
}
void LightSource::updatePhysics(float frametime)
{
	return;
}

float LightSource::ComputeIntersection(glm::vec3 origin, glm::vec3 direction)
{
	return std::numeric_limits<float>::max();
}

glm::vec3 LightSource::compute_diffuse(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::vec3 LightSource::compute_specular(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::vec3 LightSource::compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}