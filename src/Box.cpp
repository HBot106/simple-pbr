#include "Box.hpp"

Box::Box(glm::vec3 box_min, glm::vec3 box_max, glm::vec3 box_pigment, struct Finish_parseObject box_finish, glm::mat4 box_model)
{
	type = "box";
	min = box_min;
	max = box_max;
	pigment = box_pigment;
	ambient = box_finish.ambient;
	diffuse = box_finish.diffuse;
	specular = box_finish.specular;
	roughness = box_finish.roughness;
	reflection = box_finish.reflection;
	refraction = box_finish.filter;
	ior = box_finish.ior;
	filter = box_finish.filter;
	model = box_model;
	modelInverse = glm::inverse(model);
	modelInverseTranspose = glm::transpose(modelInverse);
	boundingBox.resetToPoint(min);
	boundingBox.addPoint(max);
}

Box::Box(const Box &obj)
{
	type = obj.type;
	min = obj.min;
	max = obj.max;
	pigment = obj.pigment;
	ambient = obj.ambient;
	diffuse = obj.diffuse;
	specular = obj.specular;
	roughness = obj.roughness;
	reflection = obj.reflection;
	refraction = obj.refraction;
	ior = obj.ior;
	filter = obj.filter;
	model = obj.model;
	modelInverse = glm::inverse(model);
	modelInverseTranspose = glm::transpose(modelInverse);
	boundingBox.resetToPoint(min);
	boundingBox.addPoint(max);
}

void Box::PrintAttributes()
{
	std::cout << "==============================================================================\n";
	std::cout << "Printing Box Attributes:\n";
	std::cout << "min:\t\t" << min.x << ",\t" << min.y << ",\t" << min.z << "\n";
	std::cout << "max:\t\t" << max.x << ",\t" << max.y << ",\t" << max.z << "\n";
	std::cout << "pigment:\t" << pigment.x << ",\t" << pigment.y << ",\t" << pigment.z << "\n";
	std::cout << "ambient:\t" << ambient << "\n";
	std::cout << "diffuse:\t" << diffuse << "\n";
	std::cout << "specular:\t" << specular << "\n";
	std::cout << "roughness:\t" << roughness << "\n";
	std::cout << "reflection:\t" << reflection << "\n";
	std::cout << "refraction:\t" << refraction << "\n";
	std::cout << "ior:\t\t" << ior << "\n";
	std::cout << "filter:\t\t" << filter << "\n";
	std::cout << "model:\t\t\t" << glm::to_string(model) << "\n";
	std::cout << "modelInverse:\t\t" << glm::to_string(modelInverse) << "\n";
	std::cout << "modelInverseTranspose:\t" << glm::to_string(modelInverseTranspose) << "\n";
	std::cout << "\n";
}

aaBoundingBox Box::getAABB()
{
	return boundingBox;
}
glm::mat4 Box::getModelMatrix()
{
	return model;
}
glm::mat4 Box::getModelInverseMatrix()
{
	return modelInverse;
}
glm::mat4 Box::getModelInverseTransposeMatrix()
{
	return modelInverseTranspose;
}
std::string Box::getType()
{
	return type;
}
glm::vec3 Box::getPigment()
{
	return pigment;
}
glm::vec3 Box::getLocation()
{
	return ((max + min) / 2.0f);
}
float Box::getAmbient()
{
	return ambient;
}
float Box::getSpecular()
{
	return specular;
}
float Box::getDiffuse()
{
	return diffuse;
}
float Box::getReflection()
{
	return reflection;
}
float Box::getRefraction()
{
	return refraction;
}
float Box::getRoughness()
{
	return roughness;
}
float Box::getIor()
{
	return ior;
}
float Box::getFilter()
{
	return filter;
}
void Box::updatePhysics(float frametime)
{
	return;
}

glm::vec3 Box::getNormal(glm::vec3 intersectionPoint)
{
	glm::vec3 macroSufaceNormal;
	glm::vec4 macroSufaceNormalTransformed;
	float epsilon;
	
	epsilon = 0.0001f;

	if (glm::epsilonEqual(intersectionPoint.x, min.x, epsilon))
	{
		macroSufaceNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
	}
	else if (glm::epsilonEqual(intersectionPoint.x, max.x, epsilon))
	{
		macroSufaceNormal = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else if (glm::epsilonEqual(intersectionPoint.y, min.y, epsilon))
	{
		macroSufaceNormal = glm::vec3(0.0f, -1.0f, 0.0f);
	}
	else if (glm::epsilonEqual(intersectionPoint.y, max.y, epsilon))
	{
		macroSufaceNormal = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else if (glm::epsilonEqual(intersectionPoint.z, min.z, epsilon))
	{
		macroSufaceNormal = glm::vec3(0.0f, 0.0f, -1.0f);
	}
	else if (glm::epsilonEqual(intersectionPoint.z, max.z, epsilon))
	{
		macroSufaceNormal = glm::vec3(0.0f, 0.0f, 1.0f);
	}
  
	macroSufaceNormalTransformed = modelInverseTranspose * glm::vec4(macroSufaceNormal, 0.0f);
	macroSufaceNormal = glm::vec3(macroSufaceNormalTransformed);
	return glm::normalize(macroSufaceNormal);
}

float Box::ComputeIntersection(glm::vec3 origin, glm::vec3 direction)
{
	float largestMin, smallestMax;
	glm::vec3 tmin, tmax;

	tmin = ((min - origin) / direction);
	tmax = ((max - origin) / direction);

	// make sure mins are mins and maxes are maxes
	if (tmin.x > tmax.x)
	{
		std::swap(tmin.x, tmax.x);
	}
	if (tmin.y > tmax.y)
	{
		std::swap(tmin.y, tmax.y);
	}
	if (tmin.z > tmax.z)
	{
		std::swap(tmin.z, tmax.z);
	}
    
	// get largest min and smallest max
	smallestMax = glm::min(glm::min(tmax.x, tmax.y), tmax.z);
	largestMin = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
  
	// no hit
	if (largestMin > smallestMax)
	{
		return std::numeric_limits<float>::max();
	}
	if (smallestMax < 0.0f)
	{
		return std::numeric_limits<float>::max(); // box behind
	}
	// hit
	if (largestMin > 0.0f)
	{
		return largestMin;
	}
	else
	{
		return smallestMax;
	}
}

glm::vec3 Box::compute_diffuse(SceneObject* lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(0,0,0);
}

glm::vec3 Box::compute_specular(SceneObject* lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(0,0,0);
}

glm::vec3 Box::compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(0,0,0);
}