#include "pch.h"
#include "CS_PlayerDir.h"

void CS_PlayerDir::Parse(std::string InString)
{
	JSONDocument.Parse(InString.c_str());

	UserID = JSONDocument["UserID"].GetString();
    Dir = JSONDocument["Dir"].GetString()[0];
}

std::string CS_PlayerDir::ToString()
{
    //JSONDocument를 문자열 변환 요청
    JSONDocument.SetObject();
    JSONDocument.AddMember("UserID", UserID, JSONDocument.GetAllocator());

    rapidjson::Value dirValue;
    char dirStr[2] = { Dir, '\0' };
    dirValue.SetString(dirStr, 1, JSONDocument.GetAllocator());

    JSONDocument.AddMember("Dir", dirValue, JSONDocument.GetAllocator());

    rapidjson::StringBuffer Buffer;
    rapidjson::Writer<rapidjson::StringBuffer> Writer(Buffer);
    JSONDocument.Accept(Writer);

    return Buffer.GetString();
}

int CS_PlayerDir::Length()
{
    return (int)ToString().length();
}
