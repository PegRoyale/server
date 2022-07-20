#include "logger/logger.hpp"
#include "global/global.hpp"
#include "networking/networking.hpp"

void init()
{
	logger::init("server");

	std::printf("---------- PegRoyale Dedicated Server ----------\n\n");

	if (enet_initialize() != 0)
	{
		PRINT_ERROR("Failed to start Enet");
		PRINT_ERROR("Shutting down (%i)", -1);
		return;
	}

	networking::init();

	while (!global::shutdown)
	{
		networking::update();
	}

	networking::cleanup();
}

int __cdecl main(int argc, char* argv[])
{
	init();
	return 0;
}
