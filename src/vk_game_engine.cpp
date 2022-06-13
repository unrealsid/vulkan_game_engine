#include "vk_game_engine.h"
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <fstream>
#include "VkBootstrap.h"
#include "vk_initializers.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vk_global_data.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


void VulkanEngine::init()
{
	// We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

	_window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowExtent.width,
		_windowExtent.height,
		window_flags
	);


	//load the core Vulkan structures
	init_vulkan();

	//create the swapchain
	init_swapchain();

	init_commands();

	init_default_renderpass();

	init_framebuffers();

	init_sync_structures();

	load_images();

	init_descriptors();

	init_pipelines();

	load_meshes();

	init_scene();

	//everything went fine
	_isInitialized = true;
}
void VulkanEngine::cleanup()
{
	if (_isInitialized)
	{
		vkDestroyCommandPool(_device, _commandPool, nullptr);

		vkDestroySwapchainKHR(_device, _swapchain, nullptr);

		//destroy the main renderpass
		vkDestroyRenderPass(_device, _renderPass, nullptr);

		//destroy swapchain resources
		for (int i = 0; i < _swapchainImageViews.size(); i++)
		{
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
		}

		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);

		vkDestroyInstance(_instance, nullptr);

		SDL_DestroyWindow(_window);
	}
}

void VulkanEngine::draw()
{
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(_device, 1, &_renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &_renderFence));

	//request image from the swapchain, one second timeout
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _presentSemaphore, nullptr, &swapchainImageIndex));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(_mainCommandBuffer, 0));

	//naming it cmd for shorter writing
	VkCommandBuffer cmd = _mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = _renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = _windowExtent;
	rpInfo.framebuffer = _framebuffers[swapchainImageIndex];

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	//once we start adding rendering commands, they will go here
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _defaultPipeline);

	//update_descriptors(cmd, _renderables);
	draw_objects(cmd, _renderables.data(), _renderables.size());

	//finalize the render pass
	vkCmdEndRenderPass(cmd);
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue.
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &_presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &_renderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _renderFence));

	// this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &_swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &_renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

	//increase the number of frames drawn
	_frameNumber++;
}

void VulkanEngine::process_inputs(SDL_Event event)
{
	if (event.type == SDL_KEYDOWN)
	{
		if (event.key.keysym.sym == SDLK_w)
		{
			spaceshipMovement.z -= 0.1;
		}
		
		if (event.key.keysym.sym == SDLK_a)
		{
			spaceshipMovement.x -= 0.1;
		}
		
		if (event.key.keysym.sym == SDLK_s)
		{
			spaceshipMovement.z += 0.1;
		}
		
		if (event.key.keysym.sym == SDLK_d)
		{
			spaceshipMovement.x += 0.1;
		}

		if (event.key.keysym.sym == SDLK_q)
		{
			spaceshipRotation.x += 10;
		}

		if (event.key.keysym.sym == SDLK_e)
		{
			spaceshipRotation.x -= 10;
		}

		
	}
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	//main loop
	while (!bQuit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//close the window when user alt-f4s or clicks the X button			
			if (e.type == SDL_QUIT)
			{
				bQuit = true;
			}

			//process_inputs(e);
		}

		draw();
	}
}

void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily);
	
	//create pool for upload context
	VK_CHECK(vkCreateCommandPool(_device, &uploadCommandPoolInfo, nullptr, &_uploadContext._commandPool));

	//allocate the default command buffer that we will use for the instant commands
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_uploadContext._commandPool, 1);

	VkCommandBuffer cmd;
	VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &cmd));

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//execute the function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::submit_info(&cmd);

	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _uploadContext._uploadFence));

	vkWaitForFences(_device, 1, &_uploadContext._uploadFence, true, 9999999999);
	vkResetFences(_device, 1, &_uploadContext._uploadFence);

	//clear the command pool. This will free the command buffer too
	vkResetCommandPool(_device, _uploadContext._commandPool, 0);
}

void VulkanEngine::init_vulkan()
{
	vkb::InstanceBuilder builder;

	//make the Vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(true)
		.require_api_version(1, 1, 0)
		.use_default_debug_messenger()
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	//store the instance
	_instance = vkb_inst.instance;

	//store the debug messenger
	_debug_messenger = vkb_inst.debug_messenger;

	// get the surface of the window we opened with SDL
	SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

	//use vkbootstrap to select a GPU.
	//We want a GPU that can write to the SDL surface and supports Vulkan 1.1
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(_surface)
		.select()
		.value();

	//create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a Vulkan application
	_device = vkbDevice.device;
	_chosenGPU = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue
	_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	init_vma();

	vkGetPhysicalDeviceProperties(_chosenGPU, &_gpuProperties);

	std::cout << "The GPU has a minimum buffer alignment of " << _gpuProperties.limits.minUniformBufferOffsetAlignment << std::endl;
}

void VulkanEngine::init_vma()
{
	//initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = _chosenGPU;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);
}

void VulkanEngine::init_commands()
{
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));

	//allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_commandPool, 1);

	VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer));
}


void VulkanEngine::init_sync_structures()
{
	//create synchronization structures

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;

	//we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence));

	//for the semaphores we don't need any flags
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore));
	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore));

	VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();

	VK_CHECK(vkCreateFence(_device, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence));
}