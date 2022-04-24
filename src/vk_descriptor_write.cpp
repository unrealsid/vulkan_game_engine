#include "vk_game_engine.h"
#include <glm/gtc/matrix_transform.hpp>

void VulkanEngine::update_descriptors(VkCommandBuffer cmd)
{
	//Binding 0 in the buffer
	//Vertex
	char* vertexData;
	vmaMapMemory(_allocator, _frameData.globalFrameDataBuffer._allocation, (void**)&vertexData);

	float sine = abs(sin(_frameNumber / 120.f));

	//make a model view matrix for rendering the object
	//camera view
	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, -10.0f);

	// note that we're translating the scene in the reverse direction of where we want to move
	glm::mat4 view = glm::translate(glm::mat4{1}, camPos);

	//camera projection
	float aspectRatio = static_cast<float>(_windowExtent.width) / static_cast<float>(_windowExtent.height);

	//glm::mat4 projection = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 2.0f);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

	projection[1][1] *= -1;

	_cameraData.view = view;
	_cameraData.projection = projection;
	_cameraData.viewprojection = projection * view;

	memcpy(vertexData, &_cameraData, sizeof(GPUCameraData));

	vmaUnmapMemory(_allocator, _frameData.globalFrameDataBuffer._allocation);

	//Binding 1 in the buffer
	//Fragment
	char* fragmentData;
	vmaMapMemory(_allocator, _frameData.globalFrameDataBuffer._allocation, (void**)&fragmentData);

	GlobalData frameData;
	frameData.time.x = sine;

	fragmentData += pad_uniform_buffer_size(sizeof(GPUCameraData));
	memcpy(fragmentData, &frameData, sizeof(GlobalData));

	vmaUnmapMemory(_allocator, _frameData.globalFrameDataBuffer._allocation);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipelineLayout, 0, 1, &_frameData.globalDescriptor, 0, nullptr);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipelineLayout, 2, 1, &textureSet, 0, nullptr);
}