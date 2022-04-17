#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "vk_mesh.h"

struct Material 
{
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject 
{
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
};