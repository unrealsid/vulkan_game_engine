#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "vk_mesh.h"

#define MAX_TEXTURES 3
#define MAX_OBJECTS 3

struct Material 
{
	//Stages information
	VkPipeline pipeline;

	//Push constants and descriptor sets
	VkPipelineLayout pipelineLayout;
};

struct RenderObject 
{
	Mesh* mesh;

	Material* material;

	glm::mat4 transformMatrix;

	//The global descriptor set for this object
	VkDescriptorSet _globalDescriptorSet;

	//The texture descriptor set for this object
	VkDescriptorSet _textureDescriptorSet;

	//per object data
	VkDescriptorSet _perObjectDescriptorSet;
};