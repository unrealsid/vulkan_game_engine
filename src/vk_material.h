#pragma once

#include <vulkan/vulkan.h>

#define MAX_TEXTURES 8
#define MAX_OBJECTS 4

struct Material 
{
	//Stages information
	VkPipeline pipeline;

	//Push constants and descriptor sets
	VkPipelineLayout pipelineLayout;
};

