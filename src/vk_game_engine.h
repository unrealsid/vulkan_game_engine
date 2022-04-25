#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <SDL.h>
#include <unordered_map>
#include "vk_mem_alloc.h"
#include "vk_mesh.h"
#include "vk_frame_data.h"
#include "vk_global_data.h"
#include "vk_scene_object.h"
#include "vk_types.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1

//we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)				\

class VulkanEngine
{
public:

	bool _isInitialized{ false };
	int _frameNumber{ 0 };

	VkExtent2D _windowExtent{ 1700 , 900 };

	struct SDL_Window* _window{ nullptr };

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	void process_inputs(SDL_Event event);

	//run main loop
	void run();

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkPhysicalDevice _chosenGPU;
	VkDevice _device;
	VkSurfaceKHR _surface;

	VkSwapchainKHR _swapchain; // from other articles

	// image format expected by the windowing system
	VkFormat _swapchainImageFormat;

	//array of images from the swapchain
	std::vector<VkImage> _swapchainImages;

	//array of image-views from the swapchain
	std::vector<VkImageView> _swapchainImageViews;

	VkQueue _graphicsQueue; //queue we will submit to
	uint32_t _graphicsQueueFamily; //family of that queue

	VkCommandPool _commandPool; //the command pool for our commands
	VkCommandBuffer _mainCommandBuffer; //the buffer we will record into

	VkRenderPass _renderPass;

	std::vector<VkFramebuffer> _framebuffers;

	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

	//VMA Library allocator
	VmaAllocator _allocator;

	//Inputs for this object
	VkPipelineLayout _quadPipelineLayout;
	VkPipeline _quadPipeline;

	Mesh _quadMesh;

	Mesh _monkeyMesh;

	Mesh _spaceship;

	glm::vec3 spaceshipMovement;
	glm::vec3 spaceshipRotation;

	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	FrameData _frameData;

	GPUCameraData _cameraData;

	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorSetLayout _objectSetLayout;

	VkDescriptorPool _descriptorPool;

	VkPhysicalDeviceProperties _gpuProperties;

	UploadContext _uploadContext;

	Texture _loadedTextures[MAX_TEXTURES];
	VkDescriptorSet textureSet{ VK_NULL_HANDLE };

	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

	VkDescriptorSetLayout _singleTextureSetLayout;

	VkImageView _depthImageView;
	AllocatedImage _depthImage;

	//the format for the depth image
	VkFormat _depthFormat;

	std::vector<RenderObject> _renderables;

	std::unordered_map<std::string, Material> _materials;
	std::unordered_map<std::string, Mesh> _meshes;
	//functions

	//create material and add it to the map
	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	//returns nullptr if it can't be found
	Material* get_material(const std::string& name);

	//returns nullptr if it can't be found
	Mesh* get_mesh(const std::string& name);

	//our draw function
	void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);


private:
	void init_vulkan();

	void init_vma();

	void init_swapchain();

	void init_commands();

	void init_pipelines();

	bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);

	void create_shader_modules(VkShaderModule& outVertexShaderModule, VkShaderModule& outFragmentShaderModule);

	void init_default_renderpass();

	void init_framebuffers();

	void init_sync_structures();

	void init_descriptors();

	void init_uniform_buffer_descriptors();

	void init_texture_descriptors();

	void init_storage_buffers();

	void init_scene();

	size_t pad_uniform_buffer_size(size_t originalSize);

	void load_meshes();

	void load_images();

	void upload_mesh(Mesh& mesh);
	
	void update_descriptors(VkCommandBuffer cmd);

	void build_pipeline_layout();
};
