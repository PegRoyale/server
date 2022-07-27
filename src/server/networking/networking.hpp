#pragma once

enum class proto_t
{
	NONE = -1,
	CREATE_ROOM,
	NEW_USER,
	READY_UP,
	START_GAME,
	GET_USER_LIST,
	USE_POWEWRUP,
	DIED,
	NAME_CHANGE,
	GET_LEVEL_LIST,
	ROOMS_FULL,
	ALREADY_IN_GAME,
	CHECK_SERVER_ALIVE,
	INVALID_KEY,
	GRANT_WINNER,
};

struct player_t
{
	ENetPeer* peer;
	std::string name = "N/A";
	std::string attacking;
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
	static std::string get_username(ENetPeer* peer, int room);
	static int get_user_index(ENetPeer* peer, int room);
	static int get_room(ENetPeer* peer);
	static std::string get_ip(ENetAddress address);
	static void send_webhook(const std::string& message);
	static void check_all_ready(int room);
	static int check_winner(int room);

	static std::vector<room_t> rooms;
	static ENetAddress address;
	static ENetHost* server;
	static int max_rooms;

private:
	static bool create_room(const std::string& roomid, const std::string& key);
};
