#pragma once
#include "Packet.h"
class ChatPacket : public Packet
{
public:
	std::string UserID;
	std::string Message;
	int Gold;

	virtual void Parse(std::string InString) override;
	virtual std::string ToString() override;
	virtual int Length() override;
};

