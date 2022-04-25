#include <iostream>
#include "vk_game_engine.h"

int wmain(int argc, char* argv[])
{
	VulkanEngine engine;

	engine.init();

	engine.run();

	engine.cleanup();

	return 0;
}