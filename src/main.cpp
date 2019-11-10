#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>

#include "Tokenizer.hpp"
#include "Parser.hpp"

#include "Ray.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <atomic>
#include <thread>

glm::vec3 BACKGROUND_COLOR = glm::vec3(0.0f, 0.0f, 0.0f);
bool useFresnel = false;	// 0 or 1
bool useBeers = false;		// 0 or 1
bool useAltBRDF = false;	// 0 or 1
bool useGI = false;			// 0 or 1
int useSSAA = 1;			// 1 = 1x1 superSamples, 2 = 2x2 superSamples, 3 = 3x3 superSamples, etc...
int samplesGI = 144;
int bouncesGI = 1;

const int numThreads = 16;

std::string zeroPadNumber(int num)
{
	std::ostringstream ss;
	ss << std::setw(6) << std::setfill('0') << num;
	return ss.str();
}

glm::vec3 alignHemisphereVector(glm::vec3 sampleVector, glm::vec3 up, glm::vec3 normal)
{
  float angle;
  glm::vec3 axis;
  glm::mat4 rotationMatrix;

  if (up == normal)
  {
	  return sampleVector;
  }
  if (up == -normal)
  {
	  return -sampleVector;
  }

  angle = glm::acos(glm::dot(up, normal));
  axis = glm::normalize(glm::cross(up, normal));
  
  rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
  
  return glm::normalize(glm::vec3(rotationMatrix * glm::vec4(sampleVector, 0.0f)));
}

glm::vec3 generateCosineWeightedPoint(float u, float v)
{
  float radial, theta, x, y, z, PI;

  PI = 3.14159f;
  
  radial = glm::sqrt(u);
  theta = (2.0f * PI * v);
  
  x = (radial * glm::cos(theta));
  y = (radial * glm::sin(theta));
  z = glm::sqrt(1.0f - u);
  
  return glm::vec3(x, y, z);
}

int getMaxSquareValue(int inputValue)
{
	int numPoints, numSamples;

	numPoints = inputValue;
	numSamples = 0;

	// depending on input pick a perfect square to use for the sample count for propper stratification
	// (do no more than 256 samples)
	if (numPoints >= 256) { numSamples = 256; }
	else if (numPoints >= 225) { numSamples = 225; }
	else if (numPoints >= 196) { numSamples = 196; }
	else if (numPoints >= 169) { numSamples = 169; }
	else if (numPoints >= 144) { numSamples = 144; }
	else if (numPoints >= 121) { numSamples = 121; }
	else if (numPoints >= 100) { numSamples = 100; }
	else if (numPoints >= 81) { numSamples = 81; }
	else if (numPoints >= 64) { numSamples = 64; }
	else if (numPoints >= 49) { numSamples = 49; }
	else if (numPoints >= 36) { numSamples = 36; }
	else if (numPoints >= 25) { numSamples = 25; }
	else if (numPoints >= 16) { numSamples = 16; }
	else if (numPoints >= 4) { numSamples = 4; }
	else { numSamples = 1; }

	return numSamples;
}

glm::vec3 computeDiffuse(SceneObject* intersectionObject, SceneObject* lightSourceObject, glm::vec3 lightVector, glm::vec3 intersectionPoint)
{
	glm::vec3 diffuseLightColor, macroSufaceNormal, diffuseMaterialColor, diffuseColor;

	diffuseLightColor = static_cast<LightSource*>(lightSourceObject)->color;
	diffuseMaterialColor = intersectionObject->getPigment() * intersectionObject->getDiffuse();
	macroSufaceNormal = intersectionObject->getNormal(intersectionPoint);

	diffuseColor = (diffuseMaterialColor * (glm::abs(glm::dot(macroSufaceNormal, lightVector))) * diffuseLightColor);
	diffuseColor = glm::clamp(diffuseColor, 0.0f, std::numeric_limits<float>::max());
	return diffuseColor;
}

glm::vec3 computeSpecular(SceneObject* intersectionObject, SceneObject* lightSourceObject, glm::vec3 lightVector, glm::vec3 viewVector, glm::vec3 intersectionPoint)
{
	glm::vec3 specularColor, specularMaterialColor, macroSufaceNormal, specularLightColor, halfAngleVector;
	float shininess, roughness;

	roughness = intersectionObject->getRoughness();
	shininess = (2.0f / (glm::pow(roughness, 2.0f))) - 2.0f;
	specularLightColor = static_cast<LightSource*>(lightSourceObject)->color;
	specularMaterialColor = intersectionObject->getPigment() * intersectionObject->getSpecular();

	halfAngleVector = glm::normalize(viewVector + lightVector);
	macroSufaceNormal = intersectionObject->getNormal(intersectionPoint);

	specularColor = (specularMaterialColor * specularLightColor * (glm::pow(glm::abs(glm::dot(macroSufaceNormal, halfAngleVector)), shininess)));
	specularColor = glm::clamp(specularColor, 0.0f, std::numeric_limits<float>::max());

	return specularColor;
}

float nearestIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::vector<SceneObject*> intersectionObjectList)
{
	float t, tMin;
	glm::vec4 rayOriginTransformed, rayDirectionTransformed;

	tMin = std::numeric_limits<float>::max();

	//for each object in the master sceneObject list
	for (int sceneObject_it = 0; sceneObject_it < (int)intersectionObjectList.size(); sceneObject_it++)
	{
		rayOriginTransformed = glm::vec4(rayOrigin, 1.0f);
		rayDirectionTransformed = glm::vec4(rayDirection, 0.0f);

		rayOriginTransformed = intersectionObjectList[sceneObject_it]->getModelInverseMatrix() * rayOriginTransformed;
		rayDirectionTransformed = intersectionObjectList[sceneObject_it]->getModelInverseMatrix() * rayDirectionTransformed;

		//find the intersection between the object and the current ray
		t = intersectionObjectList[sceneObject_it]->ComputeIntersection(glm::vec3(rayOriginTransformed), glm::vec3(rayDirectionTransformed));
		if (t < tMin)
		{
			//if it is closer than previous intersections, overwrite the distance to and indiex of the object
			tMin = t;
		}
	}
	return tMin;
}

int nearestIntersectionIndex(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::vector<SceneObject*> intersectionObjectList)
{
	float t, tMin;
	glm::vec4 rayOriginTransformed, rayDirectionTransformed;

	tMin = std::numeric_limits<float>::max();
	int closestIntersectionIndex = -1;

	//for each object in the master sceneObject list
	for (int sceneObject_it = 0; sceneObject_it < (int)intersectionObjectList.size(); sceneObject_it++)
	{
		rayOriginTransformed = glm::vec4(rayOrigin, 1.0f);
		rayDirectionTransformed = glm::vec4(rayDirection, 0.0f);

		rayOriginTransformed = intersectionObjectList[sceneObject_it]->getModelInverseMatrix() * rayOriginTransformed;
		rayDirectionTransformed = intersectionObjectList[sceneObject_it]->getModelInverseMatrix() * rayDirectionTransformed;

		//find the intersection between the object and the current ray
		t = intersectionObjectList[sceneObject_it]->ComputeIntersection(glm::vec3(rayOriginTransformed), glm::vec3(rayDirectionTransformed));
		if (t < tMin)
		{
			//if it is closer than previous intersections, overwrite the distance to and indiex of the object
			tMin = t;
			closestIntersectionIndex = (int)sceneObject_it;
		}
	}
	return closestIntersectionIndex;
}

glm::vec3 raycolor(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::vector<SceneObject*>  lightSourceList, std::vector<SceneObject*> intersectionObjectList, int remainingRecursiveCalls)
{
	///------------------------------------------///
	///variables used in the lighting calculation///
	///------------------------------------------///
	//pigment is used to creat ambient, diffuse, and specular
	glm::vec3 pigmentColor;
	//ambient, diffuse, and specular Blinn Phong contributions to the local color
	glm::vec3 ambientColor, diffuseColor, specularColor;
	float shadowIntersectionDistance, objectIntersectionDistance;
	int closestIntersectionIndex;
	//local color is combined with color gathered from reflection and refraction recursive casts to generate total color which is returned
	glm::vec3 localColor, reflectionColor, transmissionColor, totalColor;
	float localContribution, reflectionContribution, transmissionContribution;
	float fininshAmbient, finishReflection, finishIor, finishFilter;
	//intersection point and its various epsilon shifted points
	glm::vec3 absoluteIntersectionPoint, epslionIntersectionPointLight, epslionIntersectionPointReflection, epslionIntersectionPointTransmission;
	glm::vec4 transformedIntersectionPoint;
	float epsilon;
	//local shading vectors
	glm::vec3 viewVector, lightVector, normalVector;
	//recursive trace vectors
	glm::vec3 reflectionVector, transmissionVector;
	//Frensel variables
	float schlickApproximation, F0;
	//Beer's Law Variables
	float alpha, e;
	glm::vec3 attenuation, absorbance;
	//Transformed Rays
	glm::vec4 rayOriginTransformed, rayDirectionTransformed;

	int numSamples, dimension;
	float step, u, v;
	glm::vec3 samplePoint, hemisphereVector, hemisphereVectorEpsilon;

	//special flag
	int isLeavingObject;

	//default values;
	alpha = 0.15f;
	e = 2.7182f;
	finishIor = 1.0f;
	finishFilter = 1.0f;
	epsilon = 0.0001;
	F0 = 0.0f;
	schlickApproximation = 0.0f;
	attenuation = glm::vec3(1.0f, 1.0f, 1.0f);
	absorbance = glm::vec3(1.0f, 1.0f, 1.0f);
	rayOriginTransformed = glm::vec4(rayOrigin, 1.0f);
	rayDirectionTransformed = glm::vec4(rayDirection, 0.0f);


	//get the distance to and index of the nearest intersected object.
	objectIntersectionDistance = nearestIntersection(rayOrigin, rayDirection, intersectionObjectList);
	closestIntersectionIndex = nearestIntersectionIndex(rayOrigin, rayDirection, intersectionObjectList);

	//before the ray hits anything, assume it doesn't hit anything; color it with the background color
	localColor = BACKGROUND_COLOR;

	///----------------------------------------------------///
	///if the ray intersects any objects in the scene (hit)///
	///----------------------------------------------------///
	if (closestIntersectionIndex >= 0)
	{
		//Transform Rays
		rayOriginTransformed = intersectionObjectList[closestIntersectionIndex]->getModelInverseMatrix() * rayOriginTransformed;
		rayDirectionTransformed = intersectionObjectList[closestIntersectionIndex]->getModelInverseMatrix() * rayDirectionTransformed;

		// solve for the intersection point, view vector, and normal
		absoluteIntersectionPoint = (rayOrigin + (rayDirection * objectIntersectionDistance));
		transformedIntersectionPoint = (rayOriginTransformed + (rayDirectionTransformed * objectIntersectionDistance));

		viewVector = glm::normalize(rayOrigin - absoluteIntersectionPoint);
		normalVector = intersectionObjectList[closestIntersectionIndex]->getNormal(glm::vec3(transformedIntersectionPoint));

		//get finish
		finishReflection = intersectionObjectList[closestIntersectionIndex]->getReflection();
		finishIor = intersectionObjectList[closestIntersectionIndex]->getIor();
		finishFilter = intersectionObjectList[closestIntersectionIndex]->getFilter();
		fininshAmbient = intersectionObjectList[closestIntersectionIndex]->getAmbient();
		pigmentColor = intersectionObjectList[closestIntersectionIndex]->getPigment();

		// ======================================================================================================
		// Ambient Light Calculation
		// ======================================================================================================

		//if we are out of recursive calls
		if (useGI && remainingRecursiveCalls >= 0)
		{
			// limit the amount of bounces to bouncesGI
			// remainingRecursiveCalls is the argument from the last recursive call
			// giRemainingBounces is the argument passed to the next recursive call
			// bouncesGI is an upper limit for giRemainingBounces 
			int giRemainingBounces = 1;
			if (remainingRecursiveCalls == 0)
			{
				giRemainingBounces = 0;
			}
			if (giRemainingBounces == 0)
			{
				samplesGI = 25;
			}

			numSamples = getMaxSquareValue(samplesGI);
			dimension = (int)glm::sqrt((float)numSamples);
			step = (1.0f / (float)dimension);

			ambientColor = glm::vec3(0.0f);

			//generate points using stratification
			for (int i = 0; i < dimension; i++)
			{
				for (int j = 0; j < dimension; j++)
				{
					u = rand() / (float)RAND_MAX;
					v = rand() / (float)RAND_MAX;

					// first addition term should give the location of start of a stratified subsection
					// second addition term should take us to a random spot in that stratified subsection
					u = (((float)i * step) + (u * step));
					v = (((float)j * step) + (v * step));

					// call the cos weighting function
					samplePoint = generateCosineWeightedPoint(u, v);
					
					hemisphereVector = glm::normalize(samplePoint);

					hemisphereVector = alignHemisphereVector(hemisphereVector, glm::vec3(0.0f, 0.0f, 1.0f), normalVector);

					hemisphereVectorEpsilon = (absoluteIntersectionPoint + (epsilon * normalVector));

					ambientColor += raycolor(hemisphereVectorEpsilon, hemisphereVector, lightSourceList, intersectionObjectList, (giRemainingBounces - 1));
				}
			}
			ambientColor = (ambientColor / (float)numSamples);
			localColor += (ambientColor);
		}
		else
		{
			// no matter what, always add the ambient color
			ambientColor = pigmentColor;
			localColor += (ambientColor * fininshAmbient);
		}
		// ======================================================================================================

		//for each light
		for (std::vector<LightSource*>::size_type lightSource_it = 0; lightSource_it != lightSourceList.size(); lightSource_it++)
		{
			//caluclate vectors and points needed for lighting
			lightVector = glm::normalize(lightSourceList[lightSource_it]->getLocation() - absoluteIntersectionPoint);
			epslionIntersectionPointLight = (absoluteIntersectionPoint + (epsilon * normalVector));

			//get the distance to the nearest occluder
			shadowIntersectionDistance = nearestIntersection(epslionIntersectionPointLight, lightVector, intersectionObjectList);

			//if this distance is greater than the distance to the light, then the point is not in shadow
			if (shadowIntersectionDistance > length(lightSourceList[lightSource_it]->getLocation() - epslionIntersectionPointLight))
			{
				//do the lighting calculation
				diffuseColor = computeDiffuse(intersectionObjectList[closestIntersectionIndex], lightSourceList[lightSource_it], lightVector, glm::vec3(transformedIntersectionPoint));
				specularColor = computeSpecular(intersectionObjectList[closestIntersectionIndex], lightSourceList[lightSource_it], lightVector, viewVector, glm::vec3(transformedIntersectionPoint));

				localColor += (diffuseColor + specularColor);
			}
		}
		///---------------------------------///
		///check recursion depth then raycast///
		///---------------------------------///
		//if we are out of recursive calls
		if (remainingRecursiveCalls < 1)
		{
			//default value for deepest recursion
			reflectionColor = BACKGROUND_COLOR;
			transmissionColor = BACKGROUND_COLOR;
		}
		//otherwise do another recursive cast
		else
		{
			///-----------------------------------------------------------------///
			///calculate reflection and refraction vectors for recurive ray cast///
			///-----------------------------------------------------------------///
			//get reflection vector and associated epsilon
			reflectionVector = (rayDirection - (2 * glm::dot(rayDirection, normalVector) * normalVector));
			epslionIntersectionPointReflection = (absoluteIntersectionPoint + (epsilon * normalVector));

			//calculate the transmission vector and associated epsilon dependending on whether it's entering or leaving a shape
			if (glm::dot(rayDirection, normalVector) < 0.0f)
			{
				//entering object transmission
				isLeavingObject = 0;
				transmissionVector = glm::normalize((1.0f / finishIor) * (rayDirection - glm::dot(rayDirection, normalVector) * normalVector) - (normalVector) * (glm::sqrt(1.0f - glm::pow((1.0f / finishIor), 2.0f) * (1.0f - glm::pow(glm::dot(rayDirection, normalVector), 2.0f)))));
				epslionIntersectionPointTransmission = (absoluteIntersectionPoint + (epsilon * -normalVector));

				float nextCastDistance = nearestIntersection(epslionIntersectionPointTransmission, transmissionVector, intersectionObjectList);

				absorbance = ((glm::vec3(1.0f, 1.0f, 1.0f) - pigmentColor) * alpha * -nextCastDistance);

				attenuation.x = glm::pow(e, absorbance.x);
				attenuation.y = glm::pow(e, absorbance.y);
				attenuation.z = glm::pow(e, absorbance.z);
			}
			else
			{
				//leaving object transmission
				isLeavingObject = 1;
				normalVector = -normalVector;
				transmissionVector = glm::normalize((finishIor / 1.0f) * (rayDirection - glm::dot(rayDirection, normalVector) * normalVector) - (normalVector) * (glm::sqrt(1.0f - glm::pow((finishIor / 1.0f), 2.0f) * (1.0f - glm::pow(glm::dot(rayDirection, normalVector), 2.0f)))));
				epslionIntersectionPointTransmission = (absoluteIntersectionPoint + (epsilon * -normalVector));
			}

			//don't do the refractive cast if the hit object is not transparent
			if (finishFilter > 0.0f)
			{
				//refraction color is the local pigment times the result of the refraction raycast

				if (isLeavingObject)
				{
					transmissionColor = raycolor(epslionIntersectionPointTransmission, transmissionVector, lightSourceList, intersectionObjectList, (remainingRecursiveCalls - 1));
					reflectionColor = BACKGROUND_COLOR;
				}
				else
				{
					transmissionColor = pigmentColor * raycolor(epslionIntersectionPointTransmission, transmissionVector, lightSourceList, intersectionObjectList, (remainingRecursiveCalls - 1));
					reflectionColor = pigmentColor * raycolor(epslionIntersectionPointReflection, reflectionVector, lightSourceList, intersectionObjectList, (remainingRecursiveCalls - 1));
				}
			}
			//don't do the reflective cast if the hit object is not reflective or transparent
			else if (finishReflection > 0.0f)
			{
				//reflection color is the local pigment times the result of the reflection raycast
				reflectionColor = pigmentColor * raycolor(epslionIntersectionPointReflection, reflectionVector, lightSourceList, intersectionObjectList, (remainingRecursiveCalls - 1));
				transmissionColor = BACKGROUND_COLOR;
			}
			else
			{
				reflectionColor = BACKGROUND_COLOR;
				transmissionColor = BACKGROUND_COLOR;
			}
		}
	}

	//solve for the three color contributions
	if (useFresnel)
	{
		//Schlicks Approximation
		F0 = (glm::pow((finishIor - 1.0f), 2.0f) / glm::pow((finishIor + 1.0f), 2.0f));
		schlickApproximation = (F0 + (1.0f - F0) * glm::pow((1.0f - glm::dot(normalVector, viewVector)), 5.0f));
	}
	else
	{
		schlickApproximation = 0.0f;
	}

	localContribution = (1.0f - finishFilter) * (1.0f - finishReflection);
	reflectionContribution = (1.0f - finishFilter) * (finishReflection)+(finishFilter) * (schlickApproximation);
	transmissionContribution = (finishFilter) * (1.0f - schlickApproximation);

	//total color is the result of our raycast
	totalColor = (localContribution * localColor) + (reflectionContribution * reflectionColor) + (transmissionContribution * transmissionColor);
	return totalColor;
}

int castCameraRays(unsigned char *data, glm::ivec2 size, Camera *sceneCam, std::vector<SceneObject*> intersectionObjectsList, std::vector<SceneObject*> lightSourceList)
{
	//variables used for generating the camera rays
	int numChannels = 3;
	float Ms, Ns, Us, Vs, Ws;
	glm::vec3 u, v, w;
	glm::vec3 rayOrigin;
	glm::vec3 rayDirection;
	glm::vec3 pixelPosition;
	glm::vec3 color;
	Ws = -1.0f;

	//for each pixel of the camera plane, (imageWidth * imageHeight) pixels.
	for (int width_it = 0; width_it < size.x; width_it++)
	{
		for (int height_it = 0; height_it < size.y; height_it++)
		{
			//color variables
			unsigned char red, green, blue;
			color = glm::vec3(0.0f, 0.0f, 0.0f);

			//for each antialiasing subpixel
			for (int subpixel_width_it = 0; subpixel_width_it < useSSAA; subpixel_width_it++)
			{
				for (int subpixel_height_it = 0; subpixel_height_it < useSSAA; subpixel_height_it++)
				{
					//pixel coordinates, (Ws is hardcoded)
					Ms = (-0.5f + ((subpixel_width_it + 0.5f) / useSSAA));
					Ns = (-0.5f + ((subpixel_height_it + 0.5f) / useSSAA));
					Us = (-0.5f + ((width_it + 0.5f + Ms) / size.x));
					Vs = (-0.5f + ((height_it + 0.5f + Ns) / size.y));

					//camera vectors
					u = sceneCam->right;
					v = sceneCam->up;
					w = -(glm::normalize(sceneCam->look_at - sceneCam->location));

					//camera origin
					rayOrigin = sceneCam->location;

					//get pixel position in worldspace
					pixelPosition = (rayOrigin + (Us * u) + (Vs * v) + (Ws * w));

					//cast ray goes from the camera origin to the pixel position in world space
					rayDirection = glm::normalize(pixelPosition - rayOrigin);

					//get the color returned by this ray
					color += raycolor(rayOrigin, rayDirection, lightSourceList, intersectionObjectsList, 6);
				}
			}

			float SSAAsquared = (float)(useSSAA * useSSAA);
			color = (color / SSAAsquared);
			color = glm::clamp(color, 0.0f, 1.0f);

			//convert color channels from 0-1 to 0-255
			red = color.x * 255;
			green = color.y * 255;
			blue = color.z * 255;

			//write color to the data array
			data[(size.x * numChannels) * (size.y - 1 - height_it) + numChannels * width_it + 0] = red;
			data[(size.x * numChannels) * (size.y - 1 - height_it) + numChannels * width_it + 1] = green;
			data[(size.x * numChannels) * (size.y - 1 - height_it) + numChannels * width_it + 2] = blue;

			int percentComplete = ((float)width_it * 100.0f / (float)size.x);

			std::cout << "RayCasting: " << percentComplete << "% Complete" << std::flush << "\r";
		}
	}
	return 0;
}

int castCameraRaysThreaded(unsigned char *data, glm::ivec2 size, Camera *sceneCam, std::vector<SceneObject*> intersectionObjectsList, std::vector<SceneObject*> lightSourceList)
{
	int const pixelCount = size.x * size.y;

	std::atomic<int> doneCount;
	std::atomic<int> currentPixel;

	doneCount = 0;
	currentPixel = 0;

	auto RenderKernel = [&](int const threadIndex)
	{
		while (true)
		{
			//variables used for generating the camera rays
			int numChannels = 3;
			float Ms, Ns, Us, Vs, Ws;
			glm::vec3 u, v, w;
			glm::vec3 rayOrigin;
			glm::vec3 rayDirection;
			glm::vec3 pixelPosition;
			glm::vec3 color;
			Ws = -1.0f;

			int pixel = currentPixel++;

			if (pixel >= pixelCount)
			{
				break;
			}

			int const x = pixel / size.y;
			int const y = pixel % size.y;

			//color variables
			unsigned char red, green, blue;
			color = glm::vec3(0.0f, 0.0f, 0.0f);

			//for each antialiasing subpixel
			for (int subpixel_width_it = 0; subpixel_width_it < useSSAA; subpixel_width_it++)
			{
				for (int subpixel_height_it = 0; subpixel_height_it < useSSAA; subpixel_height_it++)
				{
					//pixel coordinates, (Ws is hardcoded)
					Ms = (-0.5f + ((subpixel_width_it + 0.5f) / useSSAA));
					Ns = (-0.5f + ((subpixel_height_it + 0.5f) / useSSAA));
					Us = (-0.5f + ((x + 0.5f + Ms) / size.x));
					Vs = (-0.5f + ((y + 0.5f + Ns) / size.y));

					//camera vectors
					u = sceneCam->right;
					v = sceneCam->up;
					w = -(glm::normalize(sceneCam->look_at - sceneCam->location));

					//camera origin
					rayOrigin = sceneCam->location;

					//get pixel position in worldspace
					pixelPosition = (rayOrigin + (Us * u) + (Vs * v) + (Ws * w));

					//cast ray goes from the camera origin to the pixel position in world space
					rayDirection = glm::normalize(pixelPosition - rayOrigin);

					//get the color returned by this ray
					color += raycolor(rayOrigin, rayDirection, lightSourceList, intersectionObjectsList, 5);
				}
			}

			float SSAAsquared = (float)(useSSAA * useSSAA);
			color = (color / SSAAsquared);
			color = glm::clamp(color, 0.0f, 1.0f);

			//convert color channels from 0-1 to 0-255
			red = color.x * 255;
			green = color.y * 255;
			blue = color.z * 255;

			//write color to the data array
			data[(size.x * numChannels) * (size.y - 1 - y) + numChannels * x + 0] = red;
			data[(size.x * numChannels) * (size.y - 1 - y) + numChannels * x + 1] = green;
			data[(size.x * numChannels) * (size.y - 1 - y) + numChannels * x + 2] = blue;
		}

		doneCount++;

		if (threadIndex == 0)
		{
			while (doneCount < numThreads)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	};

	std::vector<std::thread> threads;
	for (int i = 0; i < numThreads; ++i)
	{
		threads.push_back(std::thread(RenderKernel, i));
	}
	for (int i = 0; i < numThreads; ++i)
	{
		threads[i].join();
	}

	return 0;
}

int raycast(std::vector<SceneObject*>  sceneObjectsList, int imageWidth, int imageHeight)
{
	//create master list of rays, light sources, and intersectable objects
	std::vector<Ray*>  castRayList;
	std::vector<SceneObject*>  lightSourceList;
	std::vector<SceneObject*>  intersectionObjectsList;

	//populate the masterlists of light sources and intersectable objects
	for (std::vector<SceneObject*>::size_type sceneObject_it = 0; sceneObject_it != sceneObjectsList.size(); sceneObject_it++)
	{
		std::string myType = sceneObjectsList[sceneObject_it]->getType();

		if (myType == "light")
		{
			lightSourceList.push_back(sceneObjectsList[sceneObject_it]);
		}
		if (myType == "sphere")
		{
			intersectionObjectsList.push_back(sceneObjectsList[sceneObject_it]);
		}
		if (myType == "plane")
		{
			intersectionObjectsList.push_back(sceneObjectsList[sceneObject_it]);
		}
		if (myType == "triangle")
		{
			intersectionObjectsList.push_back(sceneObjectsList[sceneObject_it]);
		}
		if (myType == "box")
		{
			intersectionObjectsList.push_back(sceneObjectsList[sceneObject_it]);
		}
	}

	//grab camera from first position in the master list (probably shouldn't be hardcoded)
	Camera *sceneCam = static_cast<Camera*>(sceneObjectsList[0]);

	//variables used for image writing
	const int numChannels = 3;
	const std::string fileName1 = "../output.png";
	const std::string fileName2 = "output.png";
	const glm::ivec2 size = glm::ivec2(imageWidth, imageHeight);
	unsigned char *data = new unsigned char[size.x * size.y * numChannels];

	std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

	//cast camera rays and populate "data" to be written as a PNG image.
	castCameraRaysThreaded(data, size, sceneCam, intersectionObjectsList, lightSourceList);

	std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
	std::chrono::seconds diff = std::chrono::duration_cast<std::chrono::seconds>(end - start);
	std::cout << "Rendered in " << diff.count() << " seconds." << std::endl << std::endl;

	//write the image file
	stbi_write_png(fileName1.c_str(), size.x, size.y, numChannels, data, size.x * numChannels);
	stbi_write_png(fileName2.c_str(), size.x, size.y, numChannels, data, size.x * numChannels);

	int videoFrame = 0;
	while (true)
	{
		std::string frameName = ("../video/" + zeroPadNumber(videoFrame) + ".png");
		std::cout << "Generating frame: " << frameName << std::endl;
		std::cout << ((float)videoFrame * (1.0f / 60.0f)) << " seconds of video frames completed." << std::endl;

		std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

		//cast camera rays and populate "data" to be written as a PNG image.
		castCameraRaysThreaded(data, size, sceneCam, intersectionObjectsList, lightSourceList);

		std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
		std::chrono::seconds diff = std::chrono::duration_cast<std::chrono::seconds>(end - start);
		std::cout << "Rendered in " << diff.count() << " seconds." << std::endl << std::endl;

		//use the physics engine to update sphere matrices to simulate gravity
		for (int sceneObject_it = 0; sceneObject_it < (int)intersectionObjectsList.size(); sceneObject_it++)
		{
			intersectionObjectsList[sceneObject_it]->updatePhysics(1.0f / 60.0f);
		}

		//write the image file
		stbi_write_png(frameName.c_str(), size.x, size.y, numChannels, data, size.x * numChannels);

		videoFrame++;
	}

	//cleanup data
	delete[] data;

	//raycasting complete
	return 0;
}

int pixelray(std::vector<SceneObject*>  sceneObjectsList, int imageWidth, int imageHeight, int imageX, int imageY)
{


	Camera *sceneCam = static_cast<Camera*>(sceneObjectsList[0]);

	//variables used for generating the camera rays
	float Us, Vs, Ws;
	glm::vec3 u, v, w;
	glm::vec3 rayOrigin;
	glm::vec3 rayDirection;
	glm::vec3 pixelPosition;
	Ws = -1.0f;

	//pixel coordinates, (Ws is hardcoded)
	Us = (-0.5f + ((imageX + 0.5f) / imageWidth));
	Vs = (-0.5f + ((imageY + 0.5f) / imageHeight));

	//camera vectors
	u = sceneCam->right;
	v = sceneCam->up;
	w = -(glm::normalize(sceneCam->look_at - sceneCam->location));

	//camera origin
	rayOrigin = sceneCam->location;

	//get pixel position in worldspace
	pixelPosition = (rayOrigin + (Us * u) + (Vs * v) + (Ws * w));

	//cast ray goes from the camera origin to the pixel position in world space
	rayDirection = glm::normalize(pixelPosition - rayOrigin);

	std::cout << std::setiosflags(std::ios::fixed);
	std::cout << std::setprecision(4);
	std::cout << "Pixel: [" << imageX << ", " << imageY << "] Ray: {" << rayOrigin.x << " " << rayOrigin.y << " " << rayOrigin.z << "} -> {" << rayDirection.x << " " << rayDirection.y << " " << rayDirection.z << "}" << std::endl;
	return 0;
}

int pixelcolor(std::vector<SceneObject*>  sceneObjectsList, int imageWidth, int imageHeight, int imageX, int imageY)
{
	//create master list of rays, light sources, and intersectable objects
	std::vector<Ray*>  castRayList;
	std::vector<SceneObject*>  lightSourceList;
	std::vector<SceneObject*>  intersectionObjectsList;

	for (std::vector<SceneObject*>::size_type sceneObject_it = 0; sceneObject_it != sceneObjectsList.size(); sceneObject_it++)
	{
		std::string myType = sceneObjectsList[sceneObject_it]->getType();

		if (myType == "light")
		{
			lightSourceList.push_back(sceneObjectsList[sceneObject_it]);
		}
		if (myType == "sphere")
		{
			intersectionObjectsList.push_back(static_cast<Sphere*>(sceneObjectsList[sceneObject_it]));
		}
		if (myType == "plane")
		{
			intersectionObjectsList.push_back(static_cast<Plane*>(sceneObjectsList[sceneObject_it]));
		}
	}

	Camera *sceneCam = static_cast<Camera*>(sceneObjectsList[0]);

	//variables used for generating the camera rays
	float Us, Vs, Ws;
	glm::vec3 u, v, w;
	glm::vec3 rayOrigin;
	glm::vec3 rayDirection;
	glm::vec3 pixelPosition;
	Ws = -1.0f;

	//pixel coordinates, (Ws is hardcoded)
	Us = (-0.5f + ((imageX + 0.5f) / imageWidth));
	Vs = (-0.5f + ((imageY + 0.5f) / imageHeight));

	//camera vectors
	u = sceneCam->right;
	v = sceneCam->up;
	w = -(glm::normalize(sceneCam->look_at - sceneCam->location));

	//camera origin
	rayOrigin = sceneCam->location;

	//get pixel position in worldspace
	pixelPosition = (rayOrigin + (Us * u) + (Vs * v) + (Ws * w));

	//cast ray goes from the camera origin to the pixel position in world space
	rayDirection = glm::normalize(pixelPosition - rayOrigin);

	std::string tType;
	float t, tMin;
	tMin = std::numeric_limits<float>::max();

	//for each object in the master sceneObject list
	for (std::vector<SceneObject*>::size_type sceneObject_it = 0; sceneObject_it != sceneObjectsList.size(); sceneObject_it++)
	{
		//find closest intersection between the object and the current ray
		t = sceneObjectsList[sceneObject_it]->ComputeIntersection(rayOrigin, rayDirection);
		//sceneObjectsList[k]->PrintAttributes();
		if (t < tMin)
		{
			tMin = t;
			tType = sceneObjectsList[sceneObject_it]->getType();
		}
	}

	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
	color = raycolor(rayOrigin, rayDirection, lightSourceList, intersectionObjectsList, 6);

	std::cout << std::setiosflags(std::ios::fixed);
	std::cout << std::setprecision(4);
	if (color == glm::vec3(0.0f, 0.0f, 0.0f))
	{
		std::cout << "Pixel: [" << imageX << ", " << imageY << "] Ray: {" << rayOrigin.x << " " << rayOrigin.y << " " << rayOrigin.z << "} -> {" << rayDirection.x << " " << rayDirection.y << " " << rayDirection.z << "}" << std::endl;
		std::cout << "No Hit" << std::endl;
	}
	else
	{
		std::cout << "Pixel: [" << imageX << ", " << imageY << "] Ray: {" << rayOrigin.x << " " << rayOrigin.y << " " << rayOrigin.z << "} -> {" << rayDirection.x << " " << rayDirection.y << " " << rayDirection.z << "}" << std::endl;
		std::cout << "T = " << tMin << std::endl;

		if (tType == "sphere")
		{
			std::cout << "Object Type: " << "Sphere" << std::endl;
		}
		else if (tType == "plane")
		{
			std::cout << "Object Type: " << "Plane" << std::endl;
		}
		else
		{
			std::cout << "Object Type: " << tType << std::endl;
		}
		std::cout << "BRDF: " << "Blinn-Phong" << std::endl;
		std::cout << "Color: (" << (int)(255.0f * color.x) << ", " << (int)(255.0f * color.y) << ", " << (int)(255.0f * color.z) << ")" << std::endl;
	}
	return 0;
}

int firsthit(std::vector<SceneObject*>  sceneObjectsList, int imageWidth, int imageHeight, int imageX, int imageY)
{
	Camera *sceneCam = static_cast<Camera*>(sceneObjectsList[0]);

	//variables used for generating the camera rays
	float Us, Vs, Ws;
	glm::vec3 u, v, w;
	glm::vec3 rayOrigin;
	glm::vec3 rayDirection;
	glm::vec3 pixelPosition;
	Ws = -1.0f;

	//pixel coordinates, (Ws is hardcoded)
	Us = (-0.5f + ((imageX + 0.5f) / imageWidth));
	Vs = (-0.5f + ((imageY + 0.5f) / imageHeight));

	//camera vectors
	u = sceneCam->right;
	v = sceneCam->up;
	w = -(glm::normalize(sceneCam->look_at - sceneCam->location));

	//camera origin
	rayOrigin = sceneCam->location;

	//get pixel position in worldspace
	pixelPosition = (rayOrigin + (Us * u) + (Vs * v) + (Ws * w));

	//cast ray goes from the camera origin to the pixel position in world space
	rayDirection = glm::normalize(pixelPosition - rayOrigin);

	std::string tType;
	float t, tMin;
	glm::vec3 tMinColor;
	tMin = std::numeric_limits<float>::max();

	//for each object in the master sceneObject list
	for (std::vector<SceneObject*>::size_type sceneObject_it = 0; sceneObject_it != sceneObjectsList.size(); sceneObject_it++)
	{
		//find closest intersection between the object and the current ray
		t = sceneObjectsList[sceneObject_it]->ComputeIntersection(rayOrigin, rayDirection);
		//sceneObjectsList[k]->PrintAttributes();
		if (t < tMin)
		{
			tMin = t;
			tMinColor = static_cast<Sphere*>(sceneObjectsList[sceneObject_it])->pigment;
			tType = sceneObjectsList[sceneObject_it]->getType();
		}
	}

	std::cout << std::setiosflags(std::ios::fixed);
	std::cout << std::setprecision(4);
	if (tMin >= std::numeric_limits<float>::max() - 100)
	{
		std::cout << "Pixel: [" << imageX << ", " << imageY << "] Ray: {" << rayOrigin.x << " " << rayOrigin.y << " " << rayOrigin.z << "} -> {" << rayDirection.x << " " << rayDirection.y << " " << rayDirection.z << "}" << std::endl;
		std::cout << "No Hit" << std::endl;
	}
	else
	{
		std::cout << "Pixel: [" << imageX << ", " << imageY << "] Ray: {" << rayOrigin.x << " " << rayOrigin.y << " " << rayOrigin.z << "} -> {" << rayDirection.x << " " << rayDirection.y << " " << rayDirection.z << "}" << std::endl;
		std::cout << "T = " << tMin << std::endl;
		std::cout << "Object Type: " << tType << std::endl;
		std::cout << "Color: " << tMinColor.x << " " << tMinColor.y << " " << tMinColor.z << std::endl;
	}
	return 0;
}

int sceneinfo(std::vector<SceneObject*>  sceneObjectsList)
{
	//itterate over master list and print all scene object attributes.
	for (std::vector<SceneObject*>::size_type i = 0; i != sceneObjectsList.size(); i++)
	{
		sceneObjectsList[i]->PrintAttributes();
	}
	return 0;
}

int main(int argc, char **argv)
{
	int x, y;
	std::string inputLine;

	//concatonate all arguments into one string
	for (int i = 0; i < argc; i++)
	{
		inputLine = (inputLine + ' ' + argv[i]);
	}

	//check for the presences of various flags and set the appropriate global value
	if ((int)inputLine.find("-gi") != -1)
	{
		useGI = true;
		std::cout << "gi\n";
		std::cout << useGI << "\n";
	}
	if ((int)inputLine.find("-fresnel") != -1)
	{
		useFresnel = true;
		std::cout << "fresnel\n";
		std::cout << useFresnel << "\n";
	}
	if ((int)inputLine.find("-beers") != -1)
	{
		useBeers = true;
		std::cout << "beers\n";
		std::cout << useBeers << "\n";
	}
	if ((int)inputLine.find("-altbrdf") != -1)
	{
		useAltBRDF = true;
		std::cout << "altbrdf\n";
		std::cout << useAltBRDF << "\n";
	}
	if ((int)inputLine.find("-ss=") != -1)
	{
		std::string tempLine = inputLine.substr((int)inputLine.find("-ss="), -1);
		int read = sscanf(tempLine.c_str(), "-ss=%d", &useSSAA);
		if (read != 1)
		{
			std::cout << "sscanf error\n";
		}
		std::cout << useSSAA << "\n";
	}

	//we need at least two arguments
	if (argc < 3)
	{
		std::cout << "not enough arguments given...\n";
		std::cout << "Usage:\n";
		std::cout << "raytrace <action> <input_filename>\n";
		exit(1);
	}

	// get arguments 1&2 "command" and "fileHandle"
	std::string action = (std::string)argv[1];
	std::string filename = (std::string)argv[2];

	//create master list of sceen objects
	std::vector<SceneObject*>  sceneObjectsList;

	std::ifstream File;
	File.open(filename);

	if (!File.is_open())
	{
		std::cerr << "Failed to open file '" << filename << "'\n" << std::endl;
		return 2;
	}

	std::string Contents{ std::istreambuf_iterator<char>(File), std::istreambuf_iterator<char>() }; // inefficient

	Parser p;

	try
	{
		//populate the master list using the main parse function
		TokenStream ts = Tokenizer::Tokenize(Contents);
		p.Parse(ts, sceneObjectsList);
	}
	catch (const std::exception & e)
	{
		std::cerr << "exception: " << e.what() << std::endl;
	}

	//if sceneinfo is requested call that function, otherwise more arguments are needed
	if ((int)action.find("sceneinfo") != -1)
	{
		sceneinfo(sceneObjectsList);
		exit(0);
	}

	//if there are two few argumetns
	if (argc < 5)
	{
		std::cout << "not enough arguments given...\n";
		std::cout << "Usage:\n";
		std::cout << "raytrace <action> <input_filename> <width> <height>\n";
		exit(1);
	}

	// get argument 3 "width"
	std::string widthString = (std::string)argv[3];
	std::stringstream ssWidthConverter(widthString);
	int width;
	ssWidthConverter >> width;

	// get argument 4 "height"
	std::string heightString = (std::string)argv[4];
	std::stringstream ssHeightConverter(heightString);
	int height;
	ssHeightConverter >> height;

	//if the action was render, call raycast(), otherwise more arguments are needed
	if ((int)action.find("render") != -1)
	{
		raycast(sceneObjectsList, width, height);
		exit(0);
	}

	//if there are two few argumetns
	if (argc < 7)
	{
		std::cout << "not enough arguments given...\n";
		std::cout << "Usage:\n";
		std::cout << "raytrace <action> <input_filename> <width> <height> <pixel x> <pixel y>\n";
		exit(1);
	}

	// get argument 5 "x"
	std::string xString = (std::string)argv[5];
	std::stringstream ssxConverter(xString);
	ssxConverter >> x;

	// get argument 6 "y"
	std::string yString = (std::string)argv[6];
	std::stringstream ssyConverter(yString);
	ssyConverter >> y;

	//if the action is pixelray, firsthit, or pixelcolor, call their respective functions
	if ((int)action.find("pixelray") != -1)
	{
		pixelray(sceneObjectsList, width, height, x, y);
		exit(0);
	}
	else if ((int)action.find("firsthit") != -1)
	{
		firsthit(sceneObjectsList, width, height, x, y);
		exit(0);
	}
	else if ((int)action.find("pixelcolor") != -1)
	{
		pixelcolor(sceneObjectsList, width, height, x, y);
		exit(0);
	}
	else
	{
		std::cout << "Command argument could not be interpreted.\n";
		std::cout << "Expecting raycast or sceneinfo.\n";
	}
	exit(1);
}