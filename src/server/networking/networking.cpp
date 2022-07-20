#include "networking.hpp"
#include "logger/logger.hpp"
#include "global/global.hpp"

ENetAddress networking::address;
ENetHost* networking::server;
std::vector<room_t> networking::rooms;
int networking::max_rooms = 3;

void networking::init()
{
	networking::address.host = ENET_HOST_ANY;
	networking::address.port = 23363;
	PRINT_DEBUG("Binding to %u:%u", networking::address.host, networking::address.port);

	server = enet_host_create(&networking::address, 16, 2, 0, 0);

	if (!server)
	{
		PRINT_ERROR("Server is invalid");
		PRINT_ERROR("Shutting down (%i)", 0);
		global::shutdown = true;
	}
}

void networking::update()
{
	ENetEvent evt;
	while (enet_host_service(networking::server, &evt, 1000) > 0)
	{
		switch (evt.type)
		{
			case ENET_EVENT_TYPE_RECEIVE:
			{
				networking::handle_packet(evt.packet, evt.peer);
			} break;

			case ENET_EVENT_TYPE_CONNECT:
			{
				PRINT_DEBUG("Client connected");
			} break;

			case ENET_EVENT_TYPE_DISCONNECT:
			{
				PRINT_DEBUG("Client disconnected");

				int room = -1;

				for (auto i = 0; i < networking::rooms.size(); ++i)
				{
					for (auto j = 0; j < networking::rooms[i].players.size(); ++j)
					{
						if (networking::rooms[i].players[j].peer == evt.peer)
						{
							room = i;
							break;
						}
					}
				}

				networking::remove_user(evt.peer);

				if (room != -1)
				{
					bool delete_room = true;

					for (auto i = 0; i < networking::rooms[room].players.size(); ++i)
					{
						if (networking::rooms[room].players[i].peer != 0x0)
						{
							delete_room = false;
							break;
						}
					}

					if (delete_room)
					{
						PRINT_DEBUG("Deleting room \"%s\" due to lack of players!", networking::rooms[room].id.c_str());
						networking::rooms.erase(networking::rooms.begin() + room);
					}
				}
			} break;
		}
	}
}

void networking::send_packet(proto_t proto, ENetPeer* peer, const std::string& info)
{
	std::string final_info = logger::va("proto=%i;", proto).append(info);
	ENetPacket* packet = enet_packet_create(final_info.c_str(), final_info.size() + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void networking::room_broadcast_packet(proto_t proto, int room, const std::string& info)
{
	ENetPacket* packet = enet_packet_create(info.c_str(), info.size() + 1, ENET_PACKET_FLAG_RELIABLE);

	for (auto i = 0; i < networking::rooms[room].players.size(); ++i)
	{
		auto peer = networking::rooms[room].players[i].peer;

		networking::send_packet(proto, peer);
	}

	PRINT_DEBUG("Broadcasted \"%s\"", info.c_str());
}

std::string networking::get_ip(ENetAddress address)
{
	char ip[13];
	enet_address_get_host_ip(&address, ip, sizeof(ip));
	return std::string(ip);
}

void networking::cleanup()
{
	enet_host_destroy(networking::server);
	networking::rooms.clear();
}

void networking::create_room(const std::string& roomid, const std::string& key)
{
	if (networking::rooms.size() > networking::max_rooms)
	{
		PRINT_ERROR("Room limit reached!");
		return;
	}

	room_t new_room;
	new_room.id = roomid;
	new_room.key = key;
	networking::rooms.emplace_back(new_room);

	PRINT_DEBUG("New Room Created: \"%s\"", roomid.c_str());
}

void networking::handle_packet(ENetPacket* packet, ENetPeer* peer)
{
	auto split_packet = logger::split(std::string((char*)packet->data), ";");

	proto_t proto = proto_t::NONE;

	if (split_packet[0].find("proto") != std::string::npos)
	{
		proto = (proto_t)std::stoi(logger::split(split_packet[0], "=")[1]);
	}
	else
	{
		PRINT_ERROR("Unable to find protocol information!");
		return;
	}

	if (proto != proto_t::NONE)
	{
		switch (proto)
		{
			case proto_t::READY_UP:
			{
				bool all_ready = true;
				int room = -1;

				for (auto i = 0; i < networking::rooms.size(); ++i)
				{
					for (auto j = 0; j < networking::rooms[i].players.size(); ++j)
					{
						if (networking::rooms[i].players[j].peer == peer && !networking::rooms[i].players[j].ready)
						{
							networking::rooms[i].players[j].ready = true;
							room = i;
							break;
						}
					}
				}

				for (auto i = 0; i < networking::rooms[room].players.size(); ++i)
				{
					if (!networking::rooms[room].players[i].ready)
					{
						all_ready = false;
						break;
					}
				}

				if (all_ready)
				{
					PRINT_DEBUG("Starting game in room \"%s\"", networking::rooms[room].id.c_str());
					networking::room_broadcast_packet(proto_t::START_GAME, room);
				}
			} break;

			case proto_t::CREATE_ROOM:
			{
				std::string roomid, key;

				for (auto i = 1; i < split_packet.size(); ++i)
				{
					if (split_packet[i].find("roomid") != std::string::npos)
					{
						roomid = logger::split(split_packet[i], "=")[1];
						continue;
					}
					else if (split_packet[i].find("key") != std::string::npos)
					{
						key = logger::split(split_packet[i], "=")[1];
						continue;
					}
				}

				networking::create_room(roomid, key);
			} break;

			case proto_t::NEW_USER:
			{
				std::string roomid, key, name;

				for (auto i = 1; i < split_packet.size(); ++i)
				{
					if (split_packet[i].find("roomid") != std::string::npos)
					{
						roomid = logger::split(split_packet[i], "=")[1];
						continue;
					}
					else if (split_packet[i].find("key") != std::string::npos)
					{
						key = logger::split(split_packet[i], "=")[1];
						continue;
					}
					else if (split_packet[i].find("name") != std::string::npos)
					{
						name = logger::split(split_packet[i], "=")[1];
						continue;
					}
				}

				if (roomid != "" && key != "" && name != "")
				{

					for (auto i = 0; i < networking::rooms.size(); ++i)
					{
						if (networking::rooms[i].id == roomid && networking::rooms[i].key == key)
						{
							PRINT_DEBUG("Adding new player");
							
							player_t new_player;
							new_player.peer = peer;
							new_player.name = name;
							networking::rooms[i].players.emplace_back(new_player);
							break;
						}
					}
				}

			} break;
		}
	}
}

void networking::remove_user(ENetPeer* peer)
{
	bool removed = false;

	for (auto i = 0; i < networking::rooms.size(); ++i)
	{
		for (auto j = 0; j < networking::rooms[i].players.size(); ++j)
		{
			if (networking::rooms[i].players[j].peer == peer)
			{
				networking::rooms[i].players[j] = {};
				removed = true;
				break;
			}
		}
	}

	if (!removed)
	{
		PRINT_ERROR("Unable to remove player");
	}
	else
	{
		PRINT_DEBUG("Player removed");
	}
}
