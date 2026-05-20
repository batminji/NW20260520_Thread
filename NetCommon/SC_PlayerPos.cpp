#include "pch.h"
#include "SC_PlayerPos.h"

void SC_PlayerPos::Parse(std::string InString)
{
	JSONDocument.Parse(InString.c_str());

	Players.clear();

	if (JSONDocument.HasMember("Players") && JSONDocument["Players"].IsArray())
	{
		const rapidjson::Value& Arr = JSONDocument["Players"];
		for (rapidjson::SizeType i = 0; i < Arr.Size(); ++i)
		{
			PlayerData Data;
			Data.UserID = Arr[i]["UserID"].GetString();
			Data.PlayerX = Arr[i]["PlayerX"].GetInt();
			Data.PlayerY = Arr[i]["PlayerY"].GetInt();
			Players.push_back(Data);
		}
	}
}

std::string SC_PlayerPos::ToString()
{
	JSONDocument.SetObject();
	rapidjson::Document::AllocatorType& Allocator = JSONDocument.GetAllocator();

	rapidjson::Value PlayersArray(rapidjson::kArrayType);

	for (const PlayerData& Data : Players)
	{
		rapidjson::Value PlayerObj(rapidjson::kObjectType);

		rapidjson::Value UserIDValue;
		UserIDValue.SetString(Data.UserID.c_str(), (rapidjson::SizeType)Data.UserID.length(), Allocator);

		PlayerObj.AddMember("UserID", UserIDValue, Allocator);
		PlayerObj.AddMember("PlayerX", Data.PlayerX, Allocator);
		PlayerObj.AddMember("PlayerY", Data.PlayerY, Allocator);

		PlayersArray.PushBack(PlayerObj, Allocator);
	}

	JSONDocument.AddMember("Players", PlayersArray, Allocator);

	rapidjson::StringBuffer Buffer;
	rapidjson::Writer<rapidjson::StringBuffer> Writer(Buffer);
	JSONDocument.Accept(Writer);

	return Buffer.GetString();
}

int SC_PlayerPos::Length()
{
	return (int)ToString().length();
}
