#ifndef SceneObjectH
#define SceneObjectH

#include <string>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "aaBoundingBox.hpp"

class SceneObject
{
public:
	std::string type;
	aaBoundingBox boundingBox;

	SceneObject() { type = "none"; };
	SceneObject(const SceneObject &obj) { /* type = obj.type; */ };
	~SceneObject() {};

	

	virtual glm::mat4 getModelMatrix() = 0;
	virtual glm::mat4 getModelInverseMatrix() = 0;
	virtual glm::mat4 getModelInverseTransposeMatrix() = 0;
	virtual std::string getType() = 0;
	virtual glm::vec3 getPigment() = 0;
	virtual glm::vec3 getLocation() = 0;
	virtual float getAmbient() = 0;
	virtual float getSpecular() = 0;
	virtual float getDiffuse() = 0;
	virtual float getRoughness() = 0;
	virtual float getReflection() = 0;
	virtual float getRefraction() = 0;
	virtual float getIor() = 0;
	virtual float getFilter() = 0;
	virtual glm::vec3 getNormal(glm::vec3 intersectionPoint) = 0;
	virtual aaBoundingBox getAABB() = 0;
	virtual float ComputeIntersection(glm::vec3 origin, glm::vec3 direction) = 0;
	virtual void PrintAttributes() = 0;
	virtual glm::vec3 compute_diffuse(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint) = 0;
	virtual glm::vec3 compute_specular(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint) = 0;
	virtual glm::vec3 compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint) = 0;
	virtual void updatePhysics(float frametime) = 0;
};
#endif