#ifndef RayH
#define RayH

#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SceneObject.hpp"

class Ray : public SceneObject
{
public:

	std::string type;
	glm::vec3 origin;
	glm::vec3 direction;
	aaBoundingBox boundingBox;


	Ray(glm::vec3 ray_origin, glm::vec3 ray_direction);
	Ray(const Ray &obj);
	~Ray() {};

	aaBoundingBox getAABB();

	glm::mat4 getModelMatrix();
	glm::mat4 getModelInverseMatrix();
	glm::mat4 getModelInverseTransposeMatrix();
	void PrintAttributes();
	std::string getType();
	glm::vec3 getPigment();
	glm::vec3 getLocation();
	float getAmbient();
	float getSpecular();
	float getDiffuse();
	float getReflection();
	float getRefraction();
	float getRoughness();
	float getIor();
	float getFilter();
	glm::vec3 getNormal(glm::vec3 intersectionPoint);
	float ComputeIntersection(glm::vec3 origin, glm::vec3 direction);
	glm::vec3 compute_diffuse(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint);
	glm::vec3 compute_specular(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint);
	glm::vec3 compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint);
	void updatePhysics(float frametime);
};
#endif