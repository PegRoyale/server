#include "networking.hpp"
#include "logger/logger.hpp"
#include "global/global.hpp"

ENetAddress networking::address;
ENetHost* networking::server;
std::vector<room_t> networking::rooms;
int networking::max_rooms = 1;

void networking::init()
{
	networking::address.host = ENET_HOST_ANY;
	networking::address.port = 23363;
	PRINT_INFO("Binding to %u:%u", networking::address.host, networking::address.port);

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
						PRINT_INFO("Deleting room \"%s\" due to lack of players!", networking::rooms[room].id.c_str());
						networking::rooms.erase(networking::rooms.begin() + room);
					}
					else
					{
						std::string player_list;

						for (auto i = 0; i < networking::rooms[room].players.size(); ++i)
						{
							player_list.append(logger::va("%i=%s", i, networking::rooms[room].players[i].name.c_str()));

							if (i != networking::rooms[room].players.size() - 1)
							{
								player_list.append(";");
							}
						}

						networking::room_broadcast_packet(proto_t::GET_USER_LIST, room, player_list);
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
	for (auto i = 0; i < networking::rooms[room].players.size(); ++i)
	{
		auto peer = networking::rooms[room].players[i].peer;

		networking::send_packet(proto, peer, info);
	}
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

bool networking::create_room(const std::string& roomid, const std::string& key)
{
	if (networking::rooms.size() > networking::max_rooms)
	{
		PRINT_ERROR("Room limit reached!");
		return false;
	}

	bool exists = false;

	for (auto i = 0; i < networking::rooms.size(); ++i)
	{
		if (networking::rooms[i].id == roomid)
		{
			exists = true;
			break;
		}
	}

	if (!exists)
	{
		room_t new_room;
		new_room.id = roomid;
		new_room.key = key;
		networking::rooms.emplace_back(new_room);

		PRINT_INFO("New Room Created: \"%s\"", roomid.c_str());
	}
	else
	{
		PRINT_WARNING("Room \"%s\" already exists!", roomid.c_str());
	}

	return true;
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
					PRINT_INFO("Starting game in room \"%s\"", networking::rooms[room].id.c_str());
					int max_levels = 50;
					std::string player_list;
					std::string level_list;

					for (auto i = 0; i < networking::rooms[room].players.size(); ++i)
					{
						player_list.append(logger::va("%i=%s", i, networking::rooms[room].players[i].name.c_str()));

						if (i != networking::rooms[room].players.size() - 1)
						{
							player_list.append(";");
						}
					}

					static std::random_device rd;
					static std::mt19937 mt(rd());
					static std::uniform_int_distribution stage_1(1, 5);
					static std::uniform_int_distribution stage_2(6, 10);
					static std::uniform_int_distribution stage_3(11, 15);

					for (auto i = 0; i < max_levels; ++i)
					{
						if (i < 3)
						{
							level_list.append(logger::va("%i=%i", i, stage_1(mt)));
						}
						else if (i >= 3 && i < 10)
						{
							level_list.append(logger::va("%i=%i", i, stage_2(mt)));
						}
						else if (i >= 10 && i < max_levels)
						{
							level_list.append(logger::va("%i=%i", i, stage_3(mt)));
						}

						if (i != max_levels - 1)
						{
							level_list.append(";");
						}
					}

					networking::room_broadcast_packet(proto_t::GET_USER_LIST, room, player_list);
					networking::room_broadcast_packet(proto_t::GET_LEVEL_LIST, room, level_list);
					networking::room_broadcast_packet(proto_t::START_GAME, room);
					networking::rooms[room].playing = true;
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

				if (!networking::create_room(roomid, key))
				{
					networking::send_packet(proto_t::ROOMS_FULL, peer);
					enet_peer_disconnect(peer, 0);
				}
			} break;

			case proto_t::DIED:
			{
				enet_peer_disconnect(peer, 0);
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
					if (name.size() > 12)
					{
						name = name.substr(0, 12);
					}

					for (auto i = 0; i < networking::rooms.size(); ++i)
					{
						int user_exists = 0;

						if (networking::rooms[i].id == roomid && networking::rooms[i].key == key)
						{

							if (networking::rooms[i].playing)
							{
								networking::send_packet(proto_t::ALREADY_IN_GAME, peer);
								return;
							}

						retry:
							for (auto j = 0; j < networking::rooms[i].players.size(); ++j)
							{
								if (!user_exists)
								{
									if (name == networking::rooms[i].players[j].name)
									{
										++user_exists;
										goto retry;
									}
								}
								else
								{
									if ((name + logger::va("-%i", user_exists)) == networking::rooms[i].players[j].name)
									{
										++user_exists;
										goto retry;
									}
								}
							}

							
							player_t new_player;
							new_player.peer = peer;

							if(!user_exists) new_player.name = name;
							else if(user_exists) new_player.name = (name + logger::va("-%i", user_exists));

							PRINT_INFO("Adding new player \"%s\"", new_player.name.c_str());


							networking::rooms[i].players.emplace_back(new_player);
							networking::send_packet(proto_t::NAME_CHANGE, peer, logger::va("name=%s", new_player.name.c_str()));
							break;
						}
					}
				}
				else
				{
					PRINT_ERROR("Recieved malformed new player request!");
				}

			} break;

			case proto_t::USE_POWEWRUP:
			{
				powerup_t powerup;

				for (auto i = 1; i < split_packet.size(); ++i)
				{
					if (split_packet[i].find("powerup") != std::string::npos)
					{
						powerup = (powerup_t)std::stoi(logger::split(split_packet[i], "=")[1]);
						continue;
					}
				}

				auto username = networking::get_username(peer);
				auto room = networking::get_room(peer);

				for (auto i = 0; i < networking::rooms[room].players.size(); ++i)
				{
					if (networking::rooms[room].players[i].peer == peer) continue;
					else networking::send_packet(proto_t::USE_POWEWRUP, peer, logger::va("powerup=%i;user=%s;", (int)powerup, username.c_str()));
				}
			} break;
		}
	}
}

void networking::remove_user(ENetPeer* peer)
{
	bool removed = false;
	std::string name;

	for (auto i = 0; i < networking::rooms.size(); ++i)
	{
		for (auto j = 0; j < networking::rooms[i].players.size(); ++j)
		{
			if (networking::rooms[i].players[j].peer == peer)
			{
				name = networking::rooms[i].players[j].name;
				networking::rooms[i].players.erase(networking::rooms[i].players.begin() + j);
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
		PRINT_DEBUG("Player \"%s\" removed", name.c_str());
	}
}

std::string networking::get_username(ENetPeer* peer)
{
	for (auto i = 0; i < networking::rooms.size(); ++i)
	{
		for (auto j = 0; j < networking::rooms[i].players.size(); ++j)
		{
			if (networking::rooms[i].players[j].peer == peer)
			{
				return networking::rooms[i].players[j].name;
			}
		}
	}

	return "UNKNOWN";
}

int networking::get_room(ENetPeer* peer)
{
	int room = -1;
	for (auto i = 0; i < networking::rooms.size(); ++i)
	{
		for (auto j = 0; j < networking::rooms[i].players.size(); ++j)
		{
			if (networking::rooms[i].players[j].peer == peer)
			{
				room = i;
				break;
			}
		}
	}
	return room;
}
