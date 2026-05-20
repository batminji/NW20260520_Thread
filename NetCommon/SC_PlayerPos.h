#pragma once
#include "Packet.h"
class SC_PlayerPos : public Packet
{
public:
	std::string UserID;
	int PlayerX;
	int PlayerY;

	// Inherited via IPacket
	void Parse(std::string InString) override;

	std::string ToString() override;

	int Length() override;
};

