#include "vk_game_engine.h"
#include <iostream>
#include "vk_global_data.h"
#include "vk_initializers.h"
#include "vk_shader_config.h"

void VulkanEngine::init_descriptors()
{
	//create a descriptor pool that will hold 10 uniform buffers
	std::vector<VkDescriptorPoolSize> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }, 
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 }

	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = 0;
	pool_info.maxSets = 10;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorPool);

	init_uniform_buffer_descriptors();

	init_storage_buffers();

	init_texture_descriptors();
}

void VulkanEngine::init_uniform_buffer_descriptors()
{
	//information about the binding.
	VkDescriptorSetLayoutBinding cameraBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	VkDescriptorSetLayoutBinding globalFrameDataBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	VkDescriptorSetLayoutBinding bindings[] = { cameraBinding, globalFrameDataBufferBinding };

	VkDescriptorSetLayoutCreateInfo setInfo = {};
	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setInfo.pNext = nullptr;

	//we are going to have 1 binding
	setInfo.bindingCount = 2;
	//no flags
	setInfo.flags = 0;
	//point to the camera buffer binding
	setInfo.pBindings = bindings;

	vkCreateDescriptorSetLayout(_device, &setInfo, nullptr, &_globalSetLayout);

	const size_t sceneParamBufferSize = pad_uniform_buffer_size(sizeof(GlobalData)) + pad_uniform_buffer_size(sizeof(GPUCameraData));
	_frameData.globalFrameDataBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	//allocate one descriptor set for this frame
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	//using the pool we just set
	allocInfo.descriptorPool = _descriptorPool;
	//only 1 descriptor
	allocInfo.descriptorSetCount = 1;
	//using the global data layout
	allocInfo.pSetLayouts = &_globalSetLayout;

	vkAllocateDescriptorSets(_device, &allocInfo, &_frameData.globalDescriptor);

	//information about the buffer we want to point at in the descriptor
	VkDescriptorBufferInfo cameraInfo;
	cameraInfo.buffer = _frameData.globalFrameDataBuffer._buffer;
	cameraInfo.offset = 0;
	cameraInfo.range = sizeof(GPUCameraData);

	VkDescriptorBufferInfo sceneInfo;
	//it will be the camera buffer
	sceneInfo.buffer = _frameData.globalFrameDataBuffer._buffer;
	//at 0 offset
	sceneInfo.offset = pad_uniform_buffer_size(sizeof(GPUCameraData));
	//of the size of a global data struct
	sceneInfo.range = sizeof(GlobalData);

	VkWriteDescriptorSet cameraWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _frameData.globalDescriptor, &cameraInfo, 0);

	VkWriteDescriptorSet sceneWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _frameData.globalDescriptor, &sceneInfo, 1);

	VkWriteDescriptorSet setWrites[] = { cameraWrite, sceneWrite };

	vkUpdateDescriptorSets(_device, 2, setWrites, 0, nullptr);
}

void VulkanEngine::init_texture_descriptors()
{
	VkDescriptorSetLayoutBinding layoutBindings[2];
	layoutBindings[0] = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	//another set, one that holds a single texture
	layoutBindings[1] = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 2);

	VkDescriptorSetLayoutCreateInfo set3info = {};
	set3info.bindingCount = 2;
	set3info.flags = 0;
	set3info.pNext = nullptr;
	set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set3info.pBindings = &layoutBindings[0];

	vkCreateDescriptorSetLayout(_device, &set3info, nullptr, &_singleTextureSetLayout);

	//create a sampler for the texture
	VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	VkSampler blockySampler;
	vkCreateSampler(_device, &samplerInfo, nullptr, &blockySampler);

	//allocate the descriptor set for single-texture to use on the material
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_singleTextureSetLayout;

	vkAllocateDescriptorSets(_device, &allocInfo, &textureSet);

	//write to the descriptor set so that it points to our texture
	VkDescriptorImageInfo imageBufferInfos[2];

	for (uint32_t i = 0; i < 2; ++i)
	{
		imageBufferInfos[i].sampler = nullptr;
		imageBufferInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageBufferInfos[i].imageView = _loadedTextures[i].imageView;
	}

	VkWriteDescriptorSet texture1[2];

	VkDescriptorImageInfo sampler = {};
	sampler.sampler = blockySampler;

	texture1[0] = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_SAMPLER, textureSet, &sampler, 0);
	
	texture1[1] = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, textureSet, imageBufferInfos, 1, 2);

	vkUpdateDescriptorSets(_device, 2, texture1, 0, nullptr);

	std::cout << "Hello";
}

void VulkanEngine::init_storage_buffers()
{
	VkDescriptorSetLayoutBinding objectBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	VkDescriptorSetLayoutCreateInfo objectDescriptorInfo = {};
	objectDescriptorInfo.bindingCount = 1;
	objectDescriptorInfo.flags = 0;
	objectDescriptorInfo.pNext = nullptr;
	objectDescriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	objectDescriptorInfo.pBindings = &objectBufferBinding;

	vkCreateDescriptorSetLayout(_device, &objectDescriptorInfo, nullptr, &_objectSetLayout);

	_frameData.objectBuffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VkDescriptorSetAllocateInfo objectDescAllocationInfo = {};
	objectDescAllocationInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	objectDescAllocationInfo.descriptorPool = _descriptorPool;
	objectDescAllocationInfo.descriptorSetCount = 1;
	objectDescAllocationInfo.pSetLayouts = &_objectSetLayout;

	vkAllocateDescriptorSets(_device, &objectDescAllocationInfo, &_frameData.objectDescriptor);

	VkDescriptorBufferInfo objectBufferInfo;
	objectBufferInfo.buffer = _frameData.objectBuffer._buffer;
	objectBufferInfo.offset = 0;
	objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

	VkWriteDescriptorSet objectSet = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _frameData.objectDescriptor, &objectBufferInfo, 0);

	vkUpdateDescriptorSets(_device, 1, &objectSet, 0, nullptr);
}

size_t VulkanEngine::pad_uniform_buffer_size(size_t originalSize)
{
	// Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0)
	{
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}
