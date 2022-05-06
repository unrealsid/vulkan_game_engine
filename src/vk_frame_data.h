#pragma once

#include <vulkan/vulkan.h>
#include "vk_types.h"
#include <glm/glm.hpp>

struct FrameData 
{
	//buffer that holds a single buffer to use when rendering
	AllocatedBuffer globalFrameDataBuffer;

	//Resources that point inside the global buffer
	VkDescriptorSet globalDescriptor;
	
	//buffer that holds a single buffer to use when rendering
	AllocatedBuffer perObjectFrameDataBuffer;

	//Resources that point inside the global buffer
	VkDescriptorSet perObjectDescriptor;

	//Object buffer--> stores object data
	AllocatedBuffer objectBuffer;

	//Object descriptor set points resources into the objectBUffer
	VkDescriptorSet objectDescriptor;
};