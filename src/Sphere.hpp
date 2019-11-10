#ifndef SphereH
#define SphereH

#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "SceneObject.hpp"
#include "LightSource.hpp"
#include "Parser.hpp"
#include "aaBoundingBox.hpp"

class Sphere : public SceneObject
{
public:

	glm::mat4 model;
	glm::mat4 modelInverse;
	glm::mat4 modelInverseTranspose;
	std::string type;
	glm::vec3 location;
	float radius;
	glm::vec3 pigment;
	float ambient;
	float diffuse;
	float specular;
	float roughness;
	float reflection;
	float refraction;
	float ior;
	float filter;
	aaBoundingBox boundingBox;
	glm::vec3 speed;

	Sphere(glm::vec3 sphere_location, float sphere_radius, glm::vec3 sphere_pigment, struct Finish_parseObject sphere_finish, glm::mat4 sphere_model);
	Sphere(const Sphere &obj);
	~Sphere() {};

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