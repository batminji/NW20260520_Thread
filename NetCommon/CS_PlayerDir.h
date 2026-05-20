#pragma once
#include "Packet.h"
class CS_PlayerDir : public Packet
{
public:
	std::string UserID;
	char Dir;

	// Inherited via IPacket
	void Parse(std::string InString) override;

	std::string ToString() override;

	int Length() override;
};

