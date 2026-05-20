#include "pch.h"
#include "SC_PlayerPos.h"

void SC_PlayerPos::Parse(std::string InString)
{
	JSONDocument.Parse(InString.c_str());

	UserID = JSONDocument["UserID"].GetString();
	PlayerX = JSONDocument["PlayerX"].GetInt();
	PlayerY = JSONDocument["PlayerY"].GetInt();
}

std::string SC_PlayerPos::ToString()
{
	JSONDocument.SetObject();
	JSONDocument.AddMember("UserID", UserID, JSONDocument.GetAllocator());
	JSONDocument.AddMember("PlayerX", PlayerX, JSONDocument.GetAllocator());
	JSONDocument.AddMember("PlayerY", PlayerY, JSONDocument.GetAllocator());

	rapidjson::StringBuffer Buffer;
	rapidjson::Writer<rapidjson::StringBuffer> Writer(Buffer);
	JSONDocument.Accept(Writer);

	return Buffer.GetString();
}

int SC_PlayerPos::Length()
{
	return (int)ToString().length();
}
