#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include "vk_mesh.h"
#include "vk_material.h"

struct RenderObject
{
	RenderObject();

	std::shared_ptr<Mesh> mesh;

	std::vector<std::shared_ptr<Material>> materials;

	glm::mat4 transformMatrix;

	//The global descriptor set for this object
	VkDescriptorSet _globalDescriptorSet;

	//The texture descriptor set for this object
	VkDescriptorSet _textureDescriptorSet;

	//per object data
	VkDescriptorSet _perObjectDescriptorSet;

	bool load_from_obj(const char* filename);
};