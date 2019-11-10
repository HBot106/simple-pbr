
// Copyright (C) 2018 Ian Dunn
// For conditions of distribution and use, see the LICENSE file


#pragma once

#include <limits>
#include <vector>
#include <string>

#include "../src/Camera.hpp"
#include "../src/LightSource.hpp"
#include "../src/SceneObject.hpp"
#include "../src/Sphere.hpp"
#include "../src/Plane.hpp"
#include "../src/Triangle.hpp"
#include "../src/Box.hpp"
#include "../src/aaBoundingBox.hpp"


#include "TokenStream.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Finish_parseObject
{
	float ambient = 0.0f;
	float diffuse = 0.0f;
	float specular = 0.0f;

	float roughness = 0.0f;
	float reflection = 0.0f;
	float filter = 0.0f;
	float ior = 1.0f;
};

struct Transform_parseObject
{
	enum class Type
	{
		Translate,
		Scale,
		Rotate
	};

	glm::vec3 quantity;
	Type type;
};

struct Attributes_parseObject
{
	glm::vec4 pigment;
	Finish_parseObject finish;
	std::vector<Transform_parseObject> transforms;
};

struct Object_parseObject
{
	enum class Type
	{
		Sphere,
		Plane,
		Triangle,
		Box,
		Cone
	};

	Type type;
	glm::vec3 v1, v2, v3;
	float s1 = 0, s2 = 0;
	Attributes_parseObject attributes;
};

struct Camera_parseObject
{
	glm::vec3 up, right, look_at, location;
};

struct Light_parseObject
{
	glm::vec3 position;
	glm::vec4 color;
};


/// Takes a TokenStream and parses the entire file, filling the
/// camera, lights, and objects fields with the scene information.
///
/// Any malformed or unexpected contents in the .pov file is likely
/// to throw an exception.
class Parser
{

public:

	void Parse(TokenStream & tokens, std::vector<SceneObject*> & sceneObjects);

	Camera_parseObject camera;
	std::vector<Light_parseObject> lights;
	std::vector<Object_parseObject> objects;

	static glm::vec3 ParseVector3(TokenStream & tokens);
	static glm::vec4 ParseVector4(TokenStream & tokens);
	static glm::vec4 ParseColor(TokenStream & tokens);
	static glm::vec4 ParsePigment(TokenStream & tokens);
	static Finish_parseObject ParseFinish(TokenStream & tokens);
	static Attributes_parseObject ParseAttributes(TokenStream & tokens);

	static Camera_parseObject ParseCamera(TokenStream & tokens);
	static Light_parseObject ParseLightSource(TokenStream & tokens);

	static Object_parseObject ParseSphere(TokenStream & tokens);
	static Object_parseObject ParsePlane(TokenStream & tokens);
	static Object_parseObject ParseTriangle(TokenStream & tokens);
	static Object_parseObject ParseBox(TokenStream & tokens);
	static Object_parseObject ParseCone(TokenStream & tokens);

};
