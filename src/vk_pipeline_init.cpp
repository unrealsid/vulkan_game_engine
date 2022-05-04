#include "vk_game_engine.h"
#include "vk_initializers.h"
#include "vk_pipeline_builder.h"
#include "vk_shader_config.h"
#include <fstream>
#include <iostream>
#include <assert.h>

bool VulkanEngine::load_shader_module(const char* filePath, VkShaderModule* outShaderModule)
{
	//open the file. With cursor at the end
	std::ifstream file;

	file.open(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		return false;
	}
	
	//find what the size of the file is by looking up the location of the cursor
	//because the cursor is at the end, it gives the size directly in bytes
	size_t fileSize = (size_t)file.tellg();

	//spirv expects the buffer to be on uint32, so make sure to reserve a int vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	//put file cursor at beggining
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char*)buffer.data(), fileSize);

	//now that the file is loaded into the buffer, we can close it
	file.close();

	//create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	//codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	//check that the creation goes well.
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		return false;
	}

	*outShaderModule = shaderModule;
	return true;
}

void VulkanEngine::create_shader_modules(VkShaderModule& outVertexShaderModule, VkShaderModule& outFragmentShaderModule)
{
	if (!load_shader_module(FRAG_SHADER_LOCATION, &outFragmentShaderModule))
	{
		std::cout << "Error when building the quads fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Quad fragment shader successfully loaded" << std::endl;
	}

	if (!load_shader_module(VERT_SHADER_LOCATION, &outVertexShaderModule))
	{
		std::cout << "Error when building the quad vertex shader module" << std::endl;

	}
	else
	{
		std::cout << "Quad vertex shader successfully loaded" << std::endl;
	}
}

void VulkanEngine::init_pipelines()
{
	VkShaderModule quadFragShader;

	VkShaderModule quadVertexShader;

	create_shader_modules(quadVertexShader, quadFragShader);

	build_pipeline_layout();
	
	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;

	//build the mesh pipeline
	pipelineBuilder._shaderStages.clear();
	pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, quadVertexShader));

	pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, quadFragShader));

	//vertex input controls how to read vertices from vertex buffers. We aren't using it yet
	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();

	VertexInputDescription vertexDescription = Vertex::get_vertex_description();

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.width = (float)_windowExtent.width;
	pipelineBuilder._viewport.height = (float)_windowExtent.height;
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = _windowExtent;

	//configure the rasterizer to draw filled triangles
	pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

	//we don't use multisampling, so just run the default one
	pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

	//use the triangle layout we created
	pipelineBuilder._pipelineLayout = _quadPipelineLayout;

	//default depthtesting
	pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//finally build the pipeline
	_quadPipeline = pipelineBuilder.build_pipeline(_device, _renderPass);
	create_material(_quadPipeline, _quadPipelineLayout, "quad");
};

void VulkanEngine::build_pipeline_layout()
{
	//build the pipeline layout that controls the inputs/outputs of the shader
	//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

	if (_singleTextureSetLayout != nullptr)
	{
		VkDescriptorSetLayout layouts[] = { _globalSetLayout, _objectSetLayout, _singleTextureSetLayout, _perObjectSetLayout };

		pipeline_layout_info.setLayoutCount = 4;
		pipeline_layout_info.pSetLayouts = layouts;

		//setup push constants
		VkPushConstantRange push_constant;
		//this push constant range starts at the beginning
		push_constant.offset = 0;
		//this push constant range takes up the size of a ObjectPushConstants struct
		push_constant.size = sizeof(ObjectPushConstants);
		//this push constant range is accessible only in the Fragment shader
		push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		pipeline_layout_info.pPushConstantRanges = &push_constant;
		pipeline_layout_info.pushConstantRangeCount = 1;

		VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_quadPipelineLayout));
	}
	else
	{
		VkDescriptorSetLayout layouts[] = { _globalSetLayout, _objectSetLayout };

		pipeline_layout_info.setLayoutCount = 2;
		pipeline_layout_info.pSetLayouts = layouts;

		VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_quadPipelineLayout));
	}
	
}