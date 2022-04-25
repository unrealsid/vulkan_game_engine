#include "vk_game_engine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Material* VulkanEngine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	_materials[name] = mat;
	return &_materials[name];
}

Material* VulkanEngine::get_material(const std::string& name)
{
	//search for the object, and return nullptr if not found
	auto it = _materials.find(name);

	if (it == _materials.end()) 
	{
		return nullptr;
	}
	else 
	{
		return &(*it).second;
	}
}


Mesh* VulkanEngine::get_mesh(const std::string& name)
{
	auto it = _meshes.find(name);
	if (it == _meshes.end()) 
	{
		return nullptr;
	}
	else 
	{
		return &(*it).second;
	}
}

void VulkanEngine::init_scene()
{
	RenderObject quadObject;

	quadObject.material = get_material("quad");
	quadObject.mesh = get_mesh("quad");

	quadObject.material->textureID = 1;

	auto translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(0));
	auto scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(1.1f));

	/*float aaa[16] = 
	{
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1
	};

	glm::mat4 bbb;

	memcpy(glm::value_ptr(bbb), aaa, sizeof(aaa));*/

	quadObject.transformMatrix = glm::mat4{ 1.0 };

	//_renderables.push_back(quadObject);

	//RenderObject monkey;

	//monkey.material = get_material("quad");
	//monkey.mesh = get_mesh("monkey");
	//monkey.transformMatrix = glm::mat4{ 1.0f };

	//_renderables.push_back(monkey);

	RenderObject spaceship;
	spaceship.material = get_material("quad");
	spaceship.mesh = get_mesh("spaceship");
	spaceship.material->textureID = 2;
	spaceship.transformMatrix = glm::mat4{1.0f};

	_renderables.push_back(spaceship);
}

void VulkanEngine::draw_objects(VkCommandBuffer cmd, RenderObject* first, int count)
{
	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;

	void* objectData;
	vmaMapMemory(_allocator, _frameData.objectBuffer._allocation, &objectData);

	GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

	for (int i = 0; i < count; i++)
	{
		glm::mat4 model = glm::mat4{ 1.0f };

		//model rotation
		glm::mat4 rotation = glm::rotate(glm::mat4{ 1.0f }, glm::radians(90.0f), glm::vec3(1, 0, 0));
		glm::mat4 playerShipRotation = glm::rotate(glm::mat4{ 1.0f }, glm::radians(spaceshipRotation.x), glm::vec3(0, 1, 0));

		glm::mat4 translate = glm::translate(glm::mat4{ 1.0f }, spaceshipMovement);

		glm::mat4 scaleSpaceship = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.3f));

		model = scaleSpaceship * rotation * playerShipRotation * translate;

		objectSSBO[i].modelMatrix = model;
	}

	vmaUnmapMemory(_allocator, _frameData.objectBuffer._allocation);

	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];

		//only bind the pipeline if it doesn't match with the already bound one
		if (object.material != lastMaterial) 
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
			lastMaterial = object.material;

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1, &_frameData.objectDescriptor, 0, nullptr);

			//push texture ID
			ObjectPushConstants constants;
			constants.imgID = object.material->textureID;

			vkCmdPushConstants(cmd, _quadPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), &constants);
		}

		//only bind the mesh if it's a different one from last bind
		if (object.mesh != lastMesh) 
		{
			//bind the mesh vertex buffer with offset 0
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->_vertexBuffer._buffer, &offset);
			lastMesh = object.mesh;
		}

		//we can now draw
		vkCmdDraw(cmd, object.mesh->_vertices.size(), 1, 0, i);
	}
}