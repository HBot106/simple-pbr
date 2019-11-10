#include <limits>
#include "Sphere.hpp"

Sphere::Sphere(glm::vec3 sphere_location, float sphere_radius, glm::vec3 sphere_pigment, struct Finish_parseObject sphere_finish, glm::mat4 sphere_model)
{
	type = "sphere";
	location = sphere_location;
	radius = sphere_radius;
	pigment = sphere_pigment;
	ambient = sphere_finish.ambient;
	diffuse = sphere_finish.diffuse;
	specular = sphere_finish.specular;
	roughness = sphere_finish.roughness;
	reflection = sphere_finish.reflection;
	refraction = sphere_finish.filter;
	ior = sphere_finish.ior;
	filter = sphere_finish.filter;
	model = sphere_model;
	modelInverse = glm::inverse(model);
	modelInverseTranspose = glm::transpose(modelInverse);
	boundingBox.resetToPoint(location - glm::vec3(radius));
	boundingBox.addPoint(location + glm::vec3(radius));
	speed = glm::vec3(((rand() / (float)RAND_MAX) - 0.5f) * 0.5f , 0.0f, ((rand() / (float)RAND_MAX) - 0.5f) * 0.5f);
}

Sphere::Sphere(const Sphere &obj)
{
	type = obj.type;
	location = obj.location;
	radius = obj.radius;
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
	boundingBox.resetToPoint(location - glm::vec3(radius));
	boundingBox.addPoint(location + glm::vec3(radius));
	speed = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Sphere::PrintAttributes()
{
	std::cout << "==============================================================================\n";
	std::cout << "Printing Sphere Attributes:\n";
	std::cout << "location:\t" << location.x << ",\t" << location.y << ",\t" << location.z << "\n";
	std::cout << "radius:\t\t" << radius << "\n";
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

aaBoundingBox Sphere::getAABB()
{
	return boundingBox;
}
glm::mat4 Sphere::getModelMatrix()
{
	return model;
}
glm::mat4 Sphere::getModelInverseMatrix()
{
	return modelInverse;
}
glm::mat4 Sphere::getModelInverseTransposeMatrix()
{
	return modelInverseTranspose;
}
std::string Sphere::getType()
{
	return type;
}
glm::vec3 Sphere::getPigment()
{
	return pigment;
}
glm::vec3 Sphere::getLocation()
{
	return location;
}
float Sphere::getAmbient()
{
	return ambient;
}
float Sphere::getSpecular()
{
	return specular;
}
float Sphere::getDiffuse()
{
	return diffuse;
}
float Sphere::getReflection()
{
	return reflection;
}
float Sphere::getRefraction()
{
	return refraction;
}
float Sphere::getRoughness()
{
	return roughness;
}
float Sphere::getIor()
{
	return ior;
}
float Sphere::getFilter()
{
	return filter;
}

void Sphere::updatePhysics(float frametime)
{
	// apply acceleration
	speed.y -= .5f * frametime;	// frametime is just (1.0f / framerate)

	glm::vec3 positionTransformed = glm::vec3(model * glm::vec4(location, 1.0f));

	// if the sphere goes below the "floor" reflect its speed in the y direction
	if (positionTransformed.y < -3.0f)
	{
		speed.y = glm::abs(speed.y);
	}

	// reflect the spheres if they leave a bounding box
	if (positionTransformed.x < -10.0f || positionTransformed.x > 10.0f)
	{
		speed.x = -speed.x;
	}
	if (positionTransformed.z < -10.0f || positionTransformed.z > 10.0f)
	{
		speed.z = -speed.z;
	}
	
	//new translation due to current speed
	glm::mat4 translateSpeed = glm::translate(glm::mat4(1.0f), speed);

	//update matrices
	model = translateSpeed * model;
	modelInverse = glm::inverse(model);
	modelInverseTranspose = glm::transpose(modelInverse);
}

glm::vec3 Sphere::getNormal(glm::vec3 intersectionPoint)
{
	glm::vec3 macroSufaceNormal;
	glm::vec4 macroSufaceNormalTransformed;

	macroSufaceNormal = glm::normalize(glm::vec3(intersectionPoint) - location);

	macroSufaceNormalTransformed = modelInverseTranspose * glm::vec4(macroSufaceNormal, 0.0f);

	macroSufaceNormal = glm::vec3(macroSufaceNormalTransformed);

	return glm::normalize(macroSufaceNormal);
}

float Sphere::ComputeIntersection(glm::vec3 origin, glm::vec3 direction)
{
	float A, B, C, D, t1, t2;

	A = B = C = D = t1 = t2 = 0.0f;

	A = glm::dot(direction, direction);
	B = glm::dot((2.0f * direction), (origin - location));
	C = (glm::dot((origin - location), (origin - location)) - (radius * radius));
	D = (B * B) - (4.0f * A * C);

	if (D < 0.0f)
	{
		t1 = t2 = -1.0f;
	}
	else if (D == 0.0f)
	{
		t1 = ((-B) / (2.0f * A));
		t2 = -1.0f;
	}
	else if (D > 0.0f)
	{
		t1 = (((-B) + (glm::sqrt(D))) / (2 * A));
		t2 = (((-B) - (glm::sqrt(D))) / (2 * A));
	}

	if (t1 < 0.0f)
	{
		t1 = std::numeric_limits<float>::max();
	}
	if (t2 < 0.0f)
	{
		t2 = std::numeric_limits<float>::max();
	}
	return fmin(t1, t2);
}

glm::vec3 Sphere::compute_diffuse(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint)
{
	glm::vec3 diffuseColor, diffuseMaterialColor, macrosSufaceNormal, diffuseLightColor;
	if (type == "sphere")
	{
		macrosSufaceNormal = -glm::normalize(location - intersectionPoint);
		//std::cout << "type: " << type << std::endl;
	}
	else
	{
		std::cout << "wrong sceneObject type: " << type << std::endl;
	}

	diffuseLightColor = static_cast<LightSource*>(lightSourceObject)->color;
	diffuseMaterialColor = pigment * diffuse;

	diffuseColor = (diffuseMaterialColor * (glm::dot(macrosSufaceNormal, lightVector)) * diffuseLightColor);
	diffuseColor = glm::clamp(diffuseColor, 0.0f, 1.0f);
	return diffuseColor;
}

glm::vec3 Sphere::compute_specular(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	glm::vec3 specularColor, specularMaterialColor, macrosSufaceNormal, specularLightColor, halfAngleVector;
	if (type == "sphere")
	{
		macrosSufaceNormal = -glm::normalize(location - intersectionPoint);
		//std::cout << "type: " << type << std::endl;
	}
	else
	{
		std::cout << "wrong sceneObject type: " << type << std::endl;
	}

	float shininess = (2.0f / (glm::pow(roughness, 2.0f))) - 2.0f;

	specularLightColor = static_cast<LightSource*>(lightSourceObject)->color;
	specularMaterialColor = pigment * specular;

	halfAngleVector = glm::normalize(viewVector + lightVector);

	specularColor = (specularMaterialColor * specularLightColor * (glm::pow(glm::dot(macrosSufaceNormal, halfAngleVector), shininess)));
	specularColor = glm::clamp(specularColor, 0.0f, 1.0f);
	return specularColor;
}

glm::vec3 Sphere::compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	glm::vec3 macrosSufaceNormal, lightColor, diffuseMaterialColor, color, specularMaterialColor, halfAngleVector;
	float alpha, nDotH, vDotH, vDotN, lDotH, lDotN, D, G, G1, G2, F;

	if (type == "sphere")
	{
		macrosSufaceNormal = -glm::normalize(location - intersectionPoint);
	}
	else
	{
		std::cout << "wrong sceneObject type: " << type << std::endl;
	}
	lightColor = static_cast<LightSource*>(lightSourceObject)->color;
	diffuseMaterialColor = pigment * diffuse;
	halfAngleVector = glm::normalize(viewVector + lightVector);
	alpha = glm::pow(roughness, 2.0f);

	nDotH = glm::dot(macrosSufaceNormal, halfAngleVector);

	if (nDotH > 0.0f)
	{
		D = (glm::pow(alpha, 2.0f) / (glm::pi<float>() * glm::pow((glm::pow(nDotH, 2.0f) * (glm::pow(alpha, 2.0f) - 1.0f) + 1.0f), 2.0f)));
	}
	else
	{
		D = 0;
	}

	vDotH = glm::dot(viewVector, halfAngleVector);
	vDotN = glm::dot(viewVector, macrosSufaceNormal);
	lDotH = glm::dot(lightVector, halfAngleVector);
	lDotN = glm::dot(lightVector, macrosSufaceNormal);

	if ((vDotH / vDotN) > 0.0f)
	{
		G1 = 2.0f / (1.0f + glm::pow((1.0f + alpha * alpha * ((1.0f + glm::pow(vDotN, 2.0f)) / (glm::pow(vDotN, 2.0f)))), 0.5));
	}
	else
	{
		G1 = 0.0f;
	}

	if ((lDotH / lDotN) > 0.0f)
	{
		G2 = 2.0f / (1.0f + glm::pow((1.0f + alpha * alpha * ((1.0f + glm::pow(lDotN, 2.0f)) / (glm::pow(lDotN, 2.0f)))), 0.5));
	}
	else
	{
		G2 = 0.0f;
	}

	G = G1 * G2;

	F = (0.0 + (1.0) * glm::pow((1.0f - (vDotH)), 2.0f));

	specularMaterialColor = ((D * G * F) / (4.0f * glm::dot(macrosSufaceNormal, lightVector) * glm::dot(macrosSufaceNormal, viewVector))) * pigment;

	color = (lightColor * glm::dot(macrosSufaceNormal, lightVector) * ((diffuse * diffuseMaterialColor) + (specular * specularMaterialColor)));
	color = glm::clamp(color, 0.0f, 1.0f);
	return specularMaterialColor;
}