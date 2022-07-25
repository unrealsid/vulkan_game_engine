#include "vk_game_engine.h"
#include "vk_draw_init_offscreen.h"
#include <vector>

void VulkanEngine::init_offscreen_pipeline(PipelineBuilder pipelineBuilder)
{
	// Offscreen
	// Flip cull mode
	pipelineBuilder._rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	_offscreenPipeline = pipelineBuilder.build_pipeline(_device, offscreenPass.renderPass);
}