#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "vk_mesh.h"

struct Material 
{
	//Stages information
	VkPipeline pipeline;

	//Push constants and descriptor sets
	VkPipelineLayout pipelineLayout;

	int textureID = -1;
};

struct RenderObject 
{
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
};