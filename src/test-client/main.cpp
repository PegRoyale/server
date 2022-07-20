#include "logger/logger.hpp"
#include "global/global.hpp"

ENetAddress address;
ENetHost* client;
ENetPeer* server;

void init()
{
	logger::init("test-client");

	if (enet_initialize() != 0)
	{
		PRINT_ERROR("Failed to start Enet");
		PRINT_ERROR("Shutting down (%i)", -1);
		return;
	}

	PRINT_INFO("Enet initalized.");

	client = enet_host_create(0, 1, 2, 0, 0);

	if (!client)
	{
		PRINT_ERROR("Client is invalid");
		PRINT_ERROR("Shutting down (%i)", 0);
		global::shutdown = true;
	}

	enet_address_set_host(&address, "127.0.0.1");
	address.port = 23363;
	server = enet_host_connect(client, &address, 2, 0);

	if (!server)
	{
		PRINT_ERROR("Failed to find server");
		PRINT_ERROR("Shutting down (%i)", 1);
		global::shutdown = true;
	}


	while (!global::shutdown)
	{
		ENetEvent evt;
		while (enet_host_service(client, &evt, 1000) > 0)
		{
			switch (evt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				PRINT_INFO("Connected to the server");
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				PRINT_INFO("Disconnect from the server");
				break;
			}
		}
	}

	enet_host_destroy(client);
}

int __cdecl main(int argc, char* argv[])
{
	init();
	return 0;
}
