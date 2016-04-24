#ifndef _helperfunctions_cpp
#define _helperfunctions_cpp

#include <sb7.h>
#include <vmath.h>
#include <string>
#include <vector>
#include <cctype>
#include <iostream>
#include "lodepng.h"

static void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		//printf("%s\n", infoLog);
		OutputDebugString(infoLog);
		free(infoLog);
	}
}

static void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		//printf("%s\n", infoLog);
		OutputDebugString(infoLog);
		free(infoLog);
	}
}


static vmath::vec4 mult(vmath::mat4 m, vmath::vec4 v)
{
	return vmath::vec4(
		v[0] * (m[0][0] + m[1][0] + m[2][0] + m[3][0]),
		v[1] * (m[0][1] + m[1][1] + m[2][1] + m[3][1]),
		v[2] * (m[0][2] + m[1][2] + m[2][2] + m[3][2]),
		v[3] * (m[0][3] + m[1][3] + m[2][3] + m[3][3])
		);
}

static std::vector<int> IntsInString(std::string str)
{
	std::vector<int> toReturn;
	std::string intAsString = "";

	for (char& c : str)
	{
		if (std::isdigit(c))
		{
			intAsString += c;
		}
		else
		{
			if (intAsString != "")
			{
				toReturn.push_back(std::stoi(intAsString));
			}
			intAsString = "";
		}
	}

	if (intAsString != "")
	{
		toReturn.push_back(std::stoi(intAsString));
	}

	return toReturn;
}

static std::vector<unsigned char> loadImageFromFile(std::string filePath, unsigned * width, unsigned * height)
{
	std::vector<unsigned char> image;

	unsigned error = lodepng::decode(image, *width, *height, filePath);

	if (error != 0)
	{
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		std::vector<unsigned char> nothing(0);
		return nothing;
	}

	// Texture size must be power of two for the primitive OpenGL version this is written for. Find next power of two.
	size_t u2 = 1; while (u2 < *width) u2 *= 2;
	size_t v2 = 1; while (v2 < *height) v2 *= 2;
	// Ratio for power of two version compared to actual version, to render the non power of two image with proper size.
	double u3 = (double)*width / u2;
	double v3 = (double)*height / v2;

	// Make power of two version of the image.
	std::vector<unsigned char> floorTexture(u2 * v2 * 4);
	for (size_t y = 0; y < *height; y++)
		for (size_t x = 0; x < *width; x++)
			for (size_t c = 0; c < 4; c++)
			{
				floorTexture[4 * u2 * y + 4 * x + c] = image[4 * *width * y + 4 * x + c];
			}

	return floorTexture;
}

static float randBetween0and1()
{
	float lhs = (float)(rand() % 256);
	float randFloat = lhs / 256.0f;
	return randFloat;
}

static float sign()
{
	if (randBetween0and1() > 0.5)
	{
		return 1;
	}
	return -1;
}

static vmath::vec4 randomColor()
{
	return vmath::vec4(randBetween0and1(), randBetween0and1(), randBetween0and1(), 1.0f);
}

static vmath::vec3 randomDirection()
{
	float multiplier = 0.9f;
	float x = sign() * randBetween0and1() * multiplier;
	float y = 0.8f;
	float z = sign() * randBetween0and1() * multiplier;

	return vmath::vec3(x, y, z);
}


static float randSizeBetween(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

#endif