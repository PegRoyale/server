#pragma once

enum class proto_t
{
	NONE = -1,
	CREATE_ROOM,
	NEW_USER,
	READY_UP,
	START_GAME,
};

struct player_t
{
	ENetPeer* peer;
	std::string name = "N/A";
	bool ready = false;
	bool alive = false;
};

struct room_t
{
	std::string id, key;
	std::vector<player_t> players;
	bool playing = false;
};

class networking final
{
public:
	static void init();
	static void update();
	static void cleanup();
	static void send_packet(proto_t proto, ENetPeer* peer, const std::string& info = "");
	static void room_broadcast_packet(proto_t proto, int room, const std::string& info = "");
	static void handle_packet(ENetPacket* packet, ENetPeer* peer);
	static void remove_user(ENetPeer* peer);
	static std::string get_ip(ENetAddress address);
	static std::vector<room_t> rooms;

	static ENetAddress address;
	static ENetHost* server;
	static int max_rooms;

private:
	static void create_room(const std::string& roomid, const std::string& key);
};
