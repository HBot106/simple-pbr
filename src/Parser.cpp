
// Copyright (C) 2018 Ian Dunn
// For conditions of distribution and use, see the LICENSE file


#include "Parser.hpp"
#include "parse_error.hpp"
#include <sstream>

using namespace std;


void Parser::Parse(TokenStream & tokens, std::vector<SceneObject*> & sceneObjects)
{
	while (! tokens.empty())
	{
		const string token = tokens.pop();

		if (token == "camera")
		{
			camera = ParseCamera(tokens);
		}
		else if (token == "light_source")
		{
			lights.push_back(ParseLightSource(tokens));
		}
		else if (token == "sphere")
		{
			objects.push_back(ParseSphere(tokens));
		}
		else if (token == "triangle")
		{
			objects.push_back(ParseTriangle(tokens));
		}
		else if (token == "box")
		{
			objects.push_back(ParseBox(tokens));
		}
		else if (token == "cone")
		{
			objects.push_back(ParseCone(tokens));
		}
		else if (token == "plane")
		{
			objects.push_back(ParsePlane(tokens));
		}
		else
		{
			throw parse_error("unexpected top-level object", token);
		}
	}

	Camera *sceneCam = new Camera(camera.location, camera.up, camera.right, camera.look_at);
	sceneObjects.push_back(sceneCam);

	
	// populate light source list
	for (int i = 0; i < (int)lights.size(); i++)
	{
		LightSource *sceneLightSource = new LightSource(lights[i].position, lights[i].color);
		sceneObjects.push_back(sceneLightSource);
	}

	for (int i = 0; i < (int)objects.size(); i++)
	{
	  // build the model matrix
		glm::mat4 model = glm::mat4(1);
		for (int j = 0; j < (int)objects[i].attributes.transforms.size(); j++)
		{
			if (objects[i].attributes.transforms[j].type == Transform_parseObject::Type::Translate)
			{
				model = glm::translate(glm::mat4(1.0f), objects[i].attributes.transforms[j].quantity) * model;
			}
			if (objects[i].attributes.transforms[j].type == Transform_parseObject::Type::Scale)
			{
				model = glm::scale(glm::mat4(1.0f), objects[i].attributes.transforms[j].quantity) * model;
			}
			if (objects[i].attributes.transforms[j].type == Transform_parseObject::Type::Rotate)
			{
				model = glm::rotate(glm::mat4(1.0f), glm::radians(objects[i].attributes.transforms[j].quantity.x), glm::vec3(1.0f, 0.0f, 0.0f)) * model;
				model = glm::rotate(glm::mat4(1.0f), glm::radians(objects[i].attributes.transforms[j].quantity.y), glm::vec3(0.0f, 1.0f, 0.0f)) * model;
				model = glm::rotate(glm::mat4(1.0f), glm::radians(objects[i].attributes.transforms[j].quantity.z), glm::vec3(0.0f, 0.0f, 1.0f)) * model;
			}
		}
		
		// get finish
		objects[i].attributes.finish.filter = objects[i].attributes.pigment.w;

    // populate object list
		if (objects[i].type == Object_parseObject::Type::Triangle)
		{
			Triangle *sceneTriangle = new Triangle(objects[i].v1, objects[i].v2, objects[i].v3, glm::vec3(objects[i].attributes.pigment), objects[i].attributes.finish, model);
			sceneObjects.push_back(sceneTriangle);
		}
		if (objects[i].type == Object_parseObject::Type::Sphere)
		{
			Sphere *sceneSphere = new Sphere(objects[i].v1, objects[i].s1, glm::vec3(objects[i].attributes.pigment), objects[i].attributes.finish, model);
			sceneObjects.push_back(sceneSphere);
		}
		if (objects[i].type == Object_parseObject::Type::Plane)
		{
			Plane *sceneTriangle = new Plane(objects[i].v1, objects[i].s1, glm::vec3(objects[i].attributes.pigment), objects[i].attributes.finish, model);
			sceneObjects.push_back(sceneTriangle);
		}
		if (objects[i].type == Object_parseObject::Type::Box)
		{
			Box *sceneBox = new Box(objects[i].v1, objects[i].v2, glm::vec3(objects[i].attributes.pigment), objects[i].attributes.finish, model);
			sceneObjects.push_back(sceneBox);
		}
	}
}

glm::vec3 Parser::ParseVector3(TokenStream & tokens)
{
	glm::vec3 v;
	tokens.require("<");
	v.x = tokens.pop_numeric();
	tokens.require(",");
	v.y = tokens.pop_numeric();
	tokens.require(",");
	v.z = tokens.pop_numeric();
	tokens.require(">");
	return v;
}

glm::vec4 Parser::ParseVector4(TokenStream & tokens)
{
	glm::vec4 v;
	tokens.require("<");
	v.x = tokens.pop_numeric();
	tokens.require(",");
	v.y = tokens.pop_numeric();
	tokens.require(",");
	v.z = tokens.pop_numeric();
	tokens.require(",");
	v.w = tokens.pop_numeric();
	tokens.require(">");
	return v;
}

glm::vec4 Parser::ParseColor(TokenStream & tokens)
{
	glm::vec4 color;

	tokens.require("color");
	string type = tokens.pop();

	if (type == "rgb")
	{
		glm::vec3 v = ParseVector3(tokens);
		color.x = v.x;
		color.y = v.y;
		color.z = v.z;
		color.w = 0.0f;
	}
	else if (type == "rgbf")
	{
		color = ParseVector4(tokens);
	}
	else
	{
		throw parse_error("unexpected color type", type);
	}

	return color;
}

glm::vec4 Parser::ParsePigment(TokenStream & tokens)
{
	glm::vec4 pigment;

	tokens.require("{");
	pigment = ParseColor(tokens);
	tokens.require("}");

	return pigment;
}

Finish_parseObject Parser::ParseFinish(TokenStream & tokens)
{
	Finish_parseObject f;

	tokens.require("{");

	while (! tokens.empty())
	{
		string token = tokens.pop();

		if (token == "ambient")
			f.ambient = tokens.pop_numeric();
		else if (token == "diffuse")
			f.diffuse = tokens.pop_numeric();
		else if (token == "specular")
			f.specular = tokens.pop_numeric();
		else if (token == "roughness")
			f.roughness = tokens.pop_numeric();
		else if (token == "ior")
			f.ior = tokens.pop_numeric();
		else if (token == "reflection")
			f.reflection = tokens.pop_numeric();
		else if (token == "refraction")
			tokens.pop_numeric();
		else if (token == "}")
			break;
		else
			throw parse_error("unexpected finish property", token);
	}

	return f;
}

Attributes_parseObject Parser::ParseAttributes(TokenStream & tokens)
{
	Attributes_parseObject a;

	while (! tokens.empty())
	{
		string token = tokens.pop();

		if (token == "pigment")
		{
			a.pigment = ParsePigment(tokens);
		}
		else if (token == "finish")
		{
			a.finish = ParseFinish(tokens);
		}
		else if (token == "translate")
		{
			Transform_parseObject t;
			t.quantity = ParseVector3(tokens);
			t.type = Transform_parseObject::Type::Translate;
			a.transforms.push_back(t);
		}
		else if (token == "rotate")
		{
			Transform_parseObject t;
			t.quantity = ParseVector3(tokens);
			t.type = Transform_parseObject::Type::Rotate;
			a.transforms.push_back(t);
		}
		else if (token == "scale")
		{
			Transform_parseObject t;
			t.quantity = ParseVector3(tokens);
			t.type = Transform_parseObject::Type::Scale;
			a.transforms.push_back(t);
		}
		else if (token == "}")
		{
			break;
		}
		else
		{
			throw parse_error("unexpected object attribute", token);
		}
	}

	return a;
}

Camera_parseObject Parser::ParseCamera(TokenStream & tokens)
{
	Camera_parseObject c;

	tokens.require("{");

	while (! tokens.empty())
	{
		string token = tokens.pop();

		if (token == "location")
			c.location = ParseVector3(tokens);
		else if (token == "look_at")
			c.look_at = ParseVector3(tokens);
		else if (token == "up")
			c.up = ParseVector3(tokens);
		else if (token == "right")
			c.right = ParseVector3(tokens);
		else if (token == "}")
			break;
		else
			throw parse_error("unexpected camera attribute", token);
	}

	return c;
}

Light_parseObject Parser::ParseLightSource(TokenStream & tokens)
{
	Light_parseObject l;

	tokens.require("{");
	l.position = ParseVector3(tokens);
	l.color = ParseColor(tokens);
	tokens.require("}");

	return l;
}

Object_parseObject Parser::ParseSphere(TokenStream & tokens)
{
	Object_parseObject s;
	s.type = Object_parseObject::Type::Sphere;

	tokens.require("{");
	s.v1 = ParseVector3(tokens);
	tokens.require(",");
	s.s1 = tokens.pop_numeric();

	s.attributes = ParseAttributes(tokens);

	return s;
}

Object_parseObject Parser::ParsePlane(TokenStream & tokens)
{
	Object_parseObject p;
	p.type = Object_parseObject::Type::Plane;

	tokens.require("{");
	p.v1 = ParseVector3(tokens);
	tokens.require(",");
	p.s1 = tokens.pop_numeric();

	p.attributes = ParseAttributes(tokens);

	return p;
}

Object_parseObject Parser::ParseTriangle(TokenStream & tokens)
{
	Object_parseObject t;
	t.type = Object_parseObject::Type::Triangle;

	tokens.require("{");
	t.v1 = ParseVector3(tokens);
	tokens.require(",");
	t.v2 = ParseVector3(tokens);
	tokens.require(",");
	t.v3 = ParseVector3(tokens);

	t.attributes = ParseAttributes(tokens);

	return t;
}

Object_parseObject Parser::ParseBox(TokenStream & tokens)
{
	Object_parseObject b;
	b.type = Object_parseObject::Type::Box;

	tokens.require("{");
	b.v1 = ParseVector3(tokens);
	tokens.require(",");
	b.v2 = ParseVector3(tokens);

	b.attributes = ParseAttributes(tokens);

	return b;
}

Object_parseObject Parser::ParseCone(TokenStream & tokens)
{
	Object_parseObject c;
	c.type = Object_parseObject::Type::Cone;

	tokens.require("{");
	c.v1 = ParseVector3(tokens);
	tokens.require(",");
	c.s1 = tokens.pop_numeric();
	tokens.require(",");
	c.v2 = ParseVector3(tokens);
	tokens.require(",");
	c.s2 = tokens.pop_numeric();

	c.attributes = ParseAttributes(tokens);

	return c;
}
