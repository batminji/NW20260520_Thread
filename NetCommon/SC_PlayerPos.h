#pragma once
#include "Packet.h"
#include <vector>

struct PlayerData {
	std::string UserID;
	int PlayerX;
	int PlayerY;
};

class SC_PlayerPos : public Packet
{
public:
	std::string UserID;
	int PlayerX;
	int PlayerY;

	std::vector<PlayerData> Players;

	// Inherited via IPacket
	void Parse(std::string InString) override;

	std::string ToString() override;

	int Length() override;
};

