#include "vk_game_engine.h"
#include <iostream>
#include "vk_initializers.h"
#include "vk_mesh.h"
#include "vk_shader_config.h"

void VulkanEngine::load_meshes()
{
	//make the array 3 vertices long
	_quadMesh._vertices.resize(6);

	//vertex positions
	//Tri 1
	_quadMesh._vertices[0].position = { -0.5f, -0.5f, 0.0f };
	_quadMesh._vertices[1].position = { 0.5f, -0.5f, 0.0f };
	_quadMesh._vertices[2].position = { 0.5f, 0.5f, 0.0f };

	//Tri2
	_quadMesh._vertices[3].position = { 0.5f, 0.5f, 0.0f };
	_quadMesh._vertices[4].position = { -0.5f,  0.5f, 0.0f };
	_quadMesh._vertices[5].position = { -0.5f,  -0.5f, 0.0f };

	//vertex colors, all green
	_quadMesh._vertices[0].color = { 1.f, 1.f, 0.0f }; //pure green
	_quadMesh._vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	_quadMesh._vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green
	_quadMesh._vertices[3].color = { 0.f, 1.f, 1.0f }; //pure green
	_quadMesh._vertices[4].color = { 0.f, 1.f, 0.0f }; //pure green
	_quadMesh._vertices[5].color = { 1.f, 1.f, 0.0f }; //pure green

	_quadMesh._vertices[0].uv = {1.0f, 0.0f};
	_quadMesh._vertices[1].uv = {0.0f, 0.0f};
	_quadMesh._vertices[2].uv = {0.0f, 1.0f};
	_quadMesh._vertices[3].uv = {0.0f, 1.0f};
	_quadMesh._vertices[4].uv = {1.0f, 1.0f};
	_quadMesh._vertices[5].uv = {1.0f, 0.0f};

	_meshes["quad"] = _quadMesh;

	//we don't care about the vertex normals
	upload_mesh(_meshes["quad"]);

	_monkeyMesh.load_from_obj(MODEL_LOCATION);

	_meshes["monkey"] = _monkeyMesh;

	upload_mesh(_meshes["monkey"]);
}

void VulkanEngine::upload_mesh(Mesh& mesh)
{
	//allocate vertex buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//this is the total size, in bytes, of the buffer we are allocating
	bufferInfo.size = mesh._vertices.size() * sizeof(Vertex);
	//this buffer is going to be used as a Vertex Buffer
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
		&mesh._vertexBuffer._buffer,
		&mesh._vertexBuffer._allocation,
		nullptr));

	//copy vertex data
	void* data;
	vmaMapMemory(_allocator, mesh._vertexBuffer._allocation, &data);

	memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

	vmaUnmapMemory(_allocator, mesh._vertexBuffer._allocation);
}