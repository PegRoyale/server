#include "logger/logger.hpp"
#include "global/global.hpp"

ENetAddress address;
ENetHost* server;
std::vector<ENetPeer*> peers;

void init()
{
	logger::init("server");

	if (enet_initialize() != 0)
	{
		PRINT_ERROR("Failed to start Enet");
		PRINT_ERROR("Shutting down (%i)", -1);
		return;
	}

	PRINT_INFO("Enet initalized.");

	address.host = ENET_HOST_ANY;
	address.port = 23363;
	PRINT_INFO("Binding to %u:%u", address.host, address.port);

	server = enet_host_create(&address, 16, 2, 0, 0);

	if (!server)
	{
		PRINT_ERROR("Server is invalid");
		PRINT_ERROR("Shutting down (%i)", 0);
		global::shutdown = true;
	}

	while (!global::shutdown)
	{
		ENetEvent evt;
		while (enet_host_service(server, &evt, 1000) > 0)
		{
			switch (evt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				PRINT_INFO("Client connected");
				peers.emplace_back(evt.peer);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				PRINT_INFO("Client disconnected");

				bool removed = false;

				for (auto i = 0; i < peers.size(); ++i)
				{
					if (evt.peer == peers[i])
					{
						PRINT_INFO("Removed peer");
						peers[i] = 0;
						removed = true;
						break;
					}
				}

				if (!removed)
				{
					PRINT_ERROR("Unable to remove peer!");
				}
				break;
			}
		}
	}

	enet_host_destroy(server);
}

int __cdecl main(int argc, char* argv[])
{
	init();
	return 0;
}
