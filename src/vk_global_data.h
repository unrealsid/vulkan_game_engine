#pragma once
#include <glm/glm.hpp>

struct GlobalData
{
	//Current time
	glm::vec4 time = glm::vec4();
	__declspec(align(16)) glm::int32 textureID;
};

//Stores camera transforms
struct GPUCameraData
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 viewprojection;
};

struct GPUObjectData
{
	glm::mat4 modelMatrix;
};