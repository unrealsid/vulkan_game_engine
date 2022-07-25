#pragma once

#include "vk_game_engine.h"

//Offscreen frame buffer properties
#define FB_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM

//Framebuffer for offscreen rendering
struct FrameBufferAttachment
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView imageView;
};

struct OffscreenPass
{
	int32_t width, height;
	VkFramebuffer framebuffer;
	FrameBufferAttachment color, depth;
	VkRenderPass renderPass;
} offscreenPass;