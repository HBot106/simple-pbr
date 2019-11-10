#include "Plane.hpp"


Plane::Plane(glm::vec3 plane_normal, float plane_distance, glm::vec3 plane_pigment, struct Finish_parseObject plane_finish, glm::mat4 plane_model)
{
	type = "plane";
	normal = plane_normal;
	distance = plane_distance;
	pigment = plane_pigment;
	ambient = plane_finish.ambient;
	diffuse = plane_finish.diffuse;
	specular = plane_finish.specular;
	roughness = plane_finish.roughness;
	reflection = plane_finish.reflection;
	refraction = plane_finish.filter;
	ior = plane_finish.ior;
	filter = plane_finish.filter;
	model = plane_model;
	modelInverse = glm::inverse(model);
	modelInverseTranspose = glm::transpose(modelInverse);
	boundingBox.resetToPoint(glm::vec3(0.0f));
}

Plane::Plane(const Plane &obj)
{
	type = obj.type;
	normal = obj.normal;
	distance = obj.distance;
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
	boundingBox.resetToPoint(glm::vec3(0.0f));
}

void Plane::PrintAttributes()
{
	std::cout << "==============================================================================\n";
	std::cout << "Printing Plane Attributes:\n";
	std::cout << "normal:\t\t" << normal.x << ",\t" << normal.y << ",\t" << normal.z << "\n";
	std::cout << "distance:\t" << distance << "\n";
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

aaBoundingBox Plane::getAABB()
{
	return boundingBox;
}
glm::mat4 Plane::getModelMatrix()
{
	return model;
}
glm::mat4 Plane::getModelInverseMatrix()
{
	return modelInverse;
}
glm::mat4 Plane::getModelInverseTransposeMatrix()
{
	return modelInverseTranspose;
}
std::string Plane::getType()
{
	return type;
}
glm::vec3 Plane::getPigment()
{
	return pigment;
}
glm::vec3 Plane::getLocation()
{
	return normal;
}
float Plane::getAmbient()
{
	return ambient;
}
float Plane::getSpecular()
{
	return specular;
}
float Plane::getDiffuse()
{
	return diffuse;
}
float  Plane::getReflection()
{
	return reflection;
}
float  Plane::getRefraction()
{
	return refraction;
}
float Plane::getRoughness()
{
	return roughness;
}
float Plane::getIor()
{
	return ior;
}
float Plane::getFilter()
{
	return filter;
}
void Plane::updatePhysics(float frametime)
{
	return;
}

glm::vec3 Plane::getNormal(glm::vec3 intersectionPoint)
{
	glm::vec3 macroSufaceNormal;
	glm::vec4 macroSufaceNormalTransformed;

	macroSufaceNormal = glm::normalize(normal);

	macroSufaceNormalTransformed = modelInverseTranspose * glm::vec4(macroSufaceNormal, 0.0f);

	macroSufaceNormal = glm::vec3(macroSufaceNormalTransformed);

	return glm::normalize(macroSufaceNormal);
}

float Plane::ComputeIntersection(glm::vec3 origin, glm::vec3 direction)
{
	float t;

	if (glm::dot(direction, normal) == 0.0f)
	{
		return std::numeric_limits<float>::max();
	}

	t = ((distance - glm::dot(origin, normal)) / glm::dot(direction, normal));

	if (t < 0.0f)
	{
		return std::numeric_limits<float>::max();
	}
	else
	{
		return t;
	}
}

glm::vec3 Plane::compute_diffuse(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint)
{
	glm::vec3 diffuseColor, diffuseMaterialColor, macrosSufaceNormal, diffuseLightColor;
	if (type == "plane")
	{
		macrosSufaceNormal = normal;
		//std::cout << "type: " << type << std::endl;
	}
	else
	{
		std::cout << "wrong sceneObject type: " << type << std::endl;
	}

	diffuseLightColor = static_cast<LightSource*>(lightSourceObject)->color;
	diffuseMaterialColor = pigment * diffuse;

	diffuseColor = (diffuseMaterialColor * (glm::dot(macrosSufaceNormal, lightVector)) * diffuseLightColor);
	//std::cout << "diffuseColor: " << diffuseColor.x << ", " << diffuseColor.y << ", " << diffuseColor.z << std::endl << std::endl;
	diffuseColor = glm::clamp(diffuseColor, 0.0f, 1.0f);
	return diffuseColor;
}

glm::vec3 Plane::compute_specular(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	glm::vec3 specularColor, specularMaterialColor, macrosSufaceNormal, specularLightColor, halfAngleVector;
	if (type == "plane")
	{
		macrosSufaceNormal = normal;
		//std::cout << "type: " << type << std::endl;
	}
	else
	{
		std::cout << "wrong sceneObject type: " << type << std::endl;
	}

	float shininess = (2.0f / (glm::pow(roughness, 2.0f))) - 2.0f;

	specularLightColor = static_cast<LightSource*>(lightSourceObject)->color;
	specularMaterialColor = pigment * specular;

	halfAngleVector = glm::normalize(viewVector - lightVector);

	specularColor = (specularMaterialColor * specularLightColor * (glm::pow(glm::dot(macrosSufaceNormal, halfAngleVector), shininess)));
	//std::cout << "specularColor: " << specularColor.x << ", " << specularColor.y << ", " << specularColor.z << std::endl << std::endl;
	specularColor = glm::clamp(specularColor, 0.0f, 1.0f);
	return specularColor;
}

glm::vec3 Plane::compute_BRDF(SceneObject*  lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	glm::vec3 macrosSufaceNormal, lightColor, diffuseMaterialColor, color, specularMaterialColor, halfAngleVector;
	float alpha, nDotH, vDotH, vDotN, lDotH, lDotN, D, G, G1, G2, F;

	if (type == "plane")
	{
		macrosSufaceNormal = normal;
		//std::cout << "type: " << type << std::endl;
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