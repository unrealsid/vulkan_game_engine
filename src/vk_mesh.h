#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "vk_types.h"	

struct VertexInputDescription
{

	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

//Vertex Data
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;

	static VertexInputDescription get_vertex_description();
};

struct Mesh
{
	std::vector<Vertex> _vertices;
	AllocatedBuffer _vertexBuffer;
};