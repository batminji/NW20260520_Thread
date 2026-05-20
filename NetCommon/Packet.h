#pragma once
#include "pch.h"

class Packet
{
public:
	virtual void Parse(std::string InString) = 0;
	virtual std::string ToString() = 0;
	virtual int Length() = 0;
};

