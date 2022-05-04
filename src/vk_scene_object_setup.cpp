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
	spaceship.transformMatrix = glm::mat4{1.0f};

	_renderables.push_back(spaceship);

	RenderObject spaceship1;
	spaceship1.material = get_material("quad");
	spaceship1.mesh = get_mesh("spaceship");
	spaceship1.transformMatrix = glm::mat4{ 1.0f };

	_renderables.push_back(spaceship1);
	
	RenderObject spaceship2;
	spaceship2.material = get_material("quad");
	spaceship2.mesh = get_mesh("spaceship");
	spaceship2.transformMatrix = glm::mat4{ 1.0f };

	_renderables.push_back(spaceship2);

	//Assign the descriptor sets
	for(auto& renderable : _renderables)
	{
		renderable._globalDescriptorSet = _frameData.globalDescriptor;
		renderable._textureDescriptorSet = textureSet;
		renderable._perObjectDescriptorSet = _frameData.perObjectDescriptor;
	}
}

void VulkanEngine::draw_objects(VkCommandBuffer cmd, RenderObject* first, int count)
{
	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;

	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];

		//UBOS
		{
			//Binding 0 in the buffer
			//Vertex
			char* vertexData;
			vmaMapMemory(_allocator, _frameData.globalFrameDataBuffer._allocation, (void**)&vertexData);

			//make a model view matrix for rendering the object
			//camera view
			glm::vec3 camPos = glm::vec3(0.0f, 0.0f, -5.0f);

			// note that we're translating the scene in the reverse direction of where we want to move
			glm::mat4 view = glm::translate(glm::mat4{ 1 }, camPos);

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

			//Binding 0 in the dynamic buffer
			//Fragment
			char* fragmentData;
			vmaMapMemory(_allocator, _frameData.perObjectFrameDataBuffer._allocation, (void**)&fragmentData);

			float sine = abs(sin(_frameNumber / 120.f));

			GlobalData frameData;
			frameData.time.x = sine;
			frameData.textureID.x = i;

			memcpy(fragmentData, &frameData, sizeof(GlobalData));

			vmaUnmapMemory(_allocator, _frameData.perObjectFrameDataBuffer._allocation);

			//offset for our scene buffer
			auto size = pad_uniform_buffer_size(sizeof(GlobalData));
			uint32_t uniform_offset = size * i;

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipelineLayout, 0, 1, &object._globalDescriptorSet /*&_frameData.globalDescriptor*/, 0, nullptr);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipelineLayout, 2, 1, &object._textureDescriptorSet /*&textureSetr*/, 0, nullptr);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipelineLayout, 3, 1, &object._perObjectDescriptorSet, 1, &uniform_offset);
		}

		//SSBO
		{
			void* objectData;
			vmaMapMemory(_allocator, _frameData.objectBuffer._allocation, &objectData);

			GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

			glm::mat4 model = glm::mat4{ 1.0f };

			//model rotation
			glm::mat4 rotation = glm::rotate(glm::mat4{ 1.0f }, glm::radians(90.0f), glm::vec3(1, 0, 0));
			/*glm::mat4 playerShipRotation = glm::rotate(glm::mat4{ 1.0f }, glm::radians(spaceshipRotation.x), glm::vec3(0, 1, 0));

			glm::mat4 translate = glm::translate(glm::mat4{ 1.0f }, spaceshipMovement);

			glm::mat4 scaleSpaceship = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.3f));

			model = scaleSpaceship * rotation * translate * playerShipRotation;*/

			auto scale = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.4f));
			auto translate = glm::translate(glm::mat4{ 1.0f }, i == 1 ? glm::vec3(-3, 0, -2) : glm::vec3(3, 0, -2));

			if (i == 2)
			{
				translate = glm::translate(glm::mat4{ 1.0f }, glm::vec3(0, 0, 3));
			}

			model = scale * rotation * translate;

			objectSSBO[i].modelMatrix = model;

			vmaUnmapMemory(_allocator, _frameData.objectBuffer._allocation);
		}

		//VBOs
		{

			//only bind the pipeline if it doesn't match with the already bound one
			if (object.material != lastMaterial)
			{
				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
				lastMaterial = object.material;

				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1, &_frameData.objectDescriptor, 0, nullptr);
			}

			//push texture ID
			//ObjectPushConstants constants;
			//constants.imgID = object.material->textureID;

			//vkCmdPushConstants(cmd, _quadPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), &constants);

			//only bind the mesh if it's a different one from last bind
			if (object.mesh != lastMesh)
			{
				//bind the mesh vertex buffer with offset 0
				VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->_vertexBuffer._buffer, &offset);
				lastMesh = object.mesh;
			}
		}
		//we can now draw
		vkCmdDraw(cmd, object.mesh->_vertices.size(), 1, 0, i);
	}
}