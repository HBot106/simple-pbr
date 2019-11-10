#include "Triangle.hpp"

Triangle::Triangle(glm::vec3 triangle_p1, glm::vec3 triangle_p2, glm::vec3 triangle_p3, glm::vec3 triangle_pigment, struct Finish_parseObject triangle_finish, glm::mat4 triangle_model)
{
	type = "triangle";
	p1 = triangle_p1;
	p2 = triangle_p2;
	p3 = triangle_p3;
	pigment = triangle_pigment;
	ambient = triangle_finish.ambient;
	diffuse = triangle_finish.diffuse;
	specular = triangle_finish.specular;
	roughness = triangle_finish.roughness;
	reflection = triangle_finish.reflection;
	refraction = triangle_finish.filter;
	ior = triangle_finish.ior;
	filter = triangle_finish.filter;
	model = triangle_model;
	modelInverse = glm::inverse(model);
	modelInverseTranspose = glm::transpose(modelInverse);
	boundingBox.resetToPoint(p1);
	boundingBox.addPoint(p2);
	boundingBox.addPoint(p3);
}

Triangle::Triangle(const Triangle &obj)
{
	type = obj.type;
	p1 = obj.p1;
	p2 = obj.p2;
	p3 = obj.p3;
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
	boundingBox.resetToPoint(p1);
	boundingBox.addPoint(p2);
	boundingBox.addPoint(p3);
}

void Triangle::PrintAttributes()
{
	std::cout << "==============================================================================\n";
	std::cout << "Printing Triangle Attributes:\n";
	std::cout << "p1:\t\t" << p1.x << ",\t" << p1.y << ",\t" << p1.z << "\n";
	std::cout << "p2:\t\t" << p2.x << ",\t" << p2.y << ",\t" << p2.z << "\n";
	std::cout << "p3:\t\t" << p3.x << ",\t" << p3.y << ",\t" << p3.z << "\n";
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

aaBoundingBox Triangle::getAABB()
{
	return boundingBox;
}
glm::mat4 Triangle::getModelMatrix()
{
	return model;
}
glm::mat4 Triangle::getModelInverseMatrix()
{
	return modelInverse;
}
glm::mat4 Triangle::getModelInverseTransposeMatrix()
{
	return modelInverseTranspose;
}
std::string Triangle::getType()
{
	return type;
}
glm::vec3 Triangle::getPigment()
{
	return pigment;
}
glm::vec3 Triangle::getLocation()
{
  return ((p1 + p2 + p3) / 3.0f);
}
float Triangle::getAmbient()
{
	return ambient;
}
float Triangle::getSpecular()
{
	return specular;
}
float Triangle::getDiffuse()
{
	return diffuse;
}
float Triangle::getReflection()
{
	return reflection;
}
float Triangle::getRefraction()
{
	return refraction;
}
float Triangle::getRoughness()
{
	return roughness;
}
float Triangle::getIor()
{
	return ior;
}
float Triangle::getFilter()
{
	return filter;
}

void Triangle::updatePhysics(float frametime)
{
	return;
}

glm::vec3 Triangle::getNormal(glm::vec3 intersectionPoint)
{
	glm::vec3 macroSufaceNormal;
	glm::vec4 macroSufaceNormalTransformed;

	macroSufaceNormal = glm::normalize(glm::cross((p2 - p1), (p3 - p1)));

	macroSufaceNormalTransformed = modelInverseTranspose * glm::vec4(macroSufaceNormal, 0.0f);

	macroSufaceNormal = glm::vec3(macroSufaceNormalTransformed);

	return glm::normalize(macroSufaceNormal);
}

float Triangle::ComputeIntersection(glm::vec3 origin, glm::vec3 direction)
{
	/*float beta, gamma;
	float t = std::numeric_limits<float>::max();
	glm::vec3 A, B, C, D;
	glm::mat3 trinagleMatrix;
	glm::mat3 tempMatrix;

	A = p1 - p2;
	B = p1 - p3;
	C = direction;
	D = p1 - origin;

	trinagleMatrix = glm::mat3(A, B, C);

	if (glm::determinant(trinagleMatrix) == 0.0f)
	{
		return t;
	}

	tempMatrix = glm::mat3(A, B, D);
	t = (glm::determinant(tempMatrix)) / (glm::determinant(trinagleMatrix));

	tempMatrix = glm::mat3(A, D, C);
	gamma = (glm::determinant(tempMatrix)) / (glm::determinant(trinagleMatrix));

	tempMatrix = glm::mat3(D, B, C);
	beta = (glm::determinant(tempMatrix)) / (glm::determinant(trinagleMatrix));

	if ((gamma < 0.0f) || (gamma > 1.0f))
	{
		return std::numeric_limits<float>::max();
	}
	else if ((beta < 0.0f) || (beta > 1.0f - gamma))
	{
		return std::numeric_limits<float>::max();
	}
	else
	{
		return t;
	}*/

	const glm::vec3 abc = p1 - p2;
	const glm::vec3 def = p1 - p3;
	const glm::vec3 ghi = direction;
	const glm::vec3 jkl = p1 - origin;

	const float ei_hf = def.y * ghi.z - ghi.y * def.z;
	const float gf_di = ghi.x * def.z - def.x * ghi.z;
	const float dh_eg = def.x * ghi.y - def.y * ghi.x;

	const float denom = abc.x * ei_hf + abc.y * gf_di + abc.z * dh_eg;

	if (denom == 0.f)
		return std::numeric_limits<float>::max();

	const float beta = (jkl.x * ei_hf + jkl.y * gf_di + jkl.z * dh_eg) / denom;

	if (beta < 0 || beta > 1)
		return std::numeric_limits<float>::max();

	const float ak_jb = abc.x * jkl.y - jkl.x * abc.y;
	const float jc_al = jkl.x * abc.z - abc.x * jkl.z;
	const float bl_kc = abc.y * jkl.z - jkl.y * abc.z;

	const float gamma = (ghi.z * ak_jb + ghi.y * jc_al + ghi.x * bl_kc) / denom;

	if (gamma < 0 || gamma > 1 - beta)
		return std::numeric_limits<float>::max();

	const float t = (-def.z * ak_jb + -def.y * jc_al + -def.x * bl_kc) / denom;

	if (t < 0)
		return std::numeric_limits<float>::max();

	return t;
}

glm::vec3 Triangle::compute_diffuse(SceneObject* lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(0,0,0);
}

glm::vec3 Triangle::compute_specular(SceneObject* lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	return glm::vec3(0,0,0);
}

glm::vec3 Triangle::compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
  return glm::vec3(0,0,0);
}