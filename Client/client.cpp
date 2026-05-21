#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ChatPacket.h"
#include "CS_PlayerDir.h"
#include "SC_PlayerPos.h"

#include <WinSock2.h>
#include <iostream>
#include <map>
#include <process.h>
#include <conio.h>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "NetCommon")

char RecvBuffer[2048] = { 0, };
char SendBuffer[1024] = { 0, };

struct PlayerInfo
{
	int PlayerX = 0;
	int PlayerY = 0;
};

std::string PlayerUserID;
std::map<std::string, PlayerInfo> AllPlayers;

std::string GenerateRandomID(int Length = 6);

unsigned WINAPI RecvThread(void* Socket);
unsigned WINAPI SendThread(void* Socket);
// unsigned WINAPI RenderThread(void* Socket);
void RenderPlayers();

int main()
{
	srand((unsigned int)time(nullptr));
	PlayerUserID = GenerateRandomID();

	WSAData wsaData;
	int retval = 0;
	retval = WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	// ServerSockAddr.sin_addr.s_addr = inet_addr("192.168.0.95");
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(35000);

	connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));


	HANDLE ThreadHandles[2];

	ThreadHandles[0] = (HANDLE)_beginthreadex(0, 0, RecvThread, &ServerSocket, 0, 0);
	ThreadHandles[1] = (HANDLE)_beginthreadex(0, 0, SendThread, &ServerSocket, 0, 0);
	// ThreadHandles[2] = (HANDLE)_beginthreadex(0, 0, RenderThread, nullptr, 0, 0);

	// Blocking
	WaitForMultipleObjects(2, ThreadHandles, FALSE, INFINITE);

	closesocket(ServerSocket);

	CloseHandle(ThreadHandles[0]);
	CloseHandle(ThreadHandles[1]);
	// CloseHandle(ThreadHandles[2]);

	WSACleanup();

	return 0;
}

std::string GenerateRandomID(int Length)
{
	const char Chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	const int CharsLen = (int)(sizeof(Chars) - 1);

	std::string Result = "user_";
	for (int i = 0; i < Length; ++i)
	{
		Result += Chars[rand() % CharsLen];
	}

	return Result;
}

unsigned __stdcall RecvThread(void* Socket)
{
	SOCKET ServerSocket = *(SOCKET*)Socket;

	while (true)
	{
		int RecvBytes = recv(ServerSocket, RecvBuffer, sizeof(RecvBuffer), 0);
		if (RecvBytes <= 0)
		{
			printf("recv fail!\n");
			break;
		}
		RecvBuffer[RecvBytes] = '\0';

		SC_PlayerPos Data;
		Data.Parse(RecvBuffer);

		for (const PlayerData& Player : Data.Players)
		{
			// std::cout << "[" << Player.UserID << "]" << " X: " << Player.PlayerX << " Y : " << Player.PlayerY << std::endl;
			AllPlayers[Player.UserID] = { Player.PlayerX , Player.PlayerY };
		}
		// std::cout << "--------------------------" << std::endl;
		RenderPlayers();
	}

	return 0;
}

unsigned __stdcall SendThread(void* Socket)
{
	SOCKET ServerSocket = *(SOCKET*)Socket;

	while (true)
	{
		if (_kbhit())
		{
			char Key = _getch();
			char Dir = ' ';

			if (Key == 'w' || Key == 'W')
			{
				Dir = 'W';
			}
			else if (Key == 's' || Key == 'S')
			{
				Dir = 'S';
			}
			else if (Key == 'a' || Key == 'A')
			{
				Dir = 'A';
			}
			else if (Key == 'd' || Key == 'D')
			{
				Dir = 'D';
			}

			if (Dir != ' ')
			{
				CS_PlayerDir PlayerDirPacket;
				PlayerDirPacket.UserID = PlayerUserID;
				PlayerDirPacket.Dir = Dir;

				std::string JSONString = PlayerDirPacket.ToString();

				int SentBytes = send(ServerSocket, JSONString.c_str(), (int)JSONString.length(), 0);
				if (SentBytes <= 0)
				{
					printf("send fail!\n");
					break;
				}
			}
		}
	}

	return 0;
}

void RenderPlayers()
{
	system("cls");
	for (const auto& Player : AllPlayers)
	{
		if (Player.second.PlayerY >= 0 && Player.second.PlayerY < 20
			&& Player.second.PlayerX >= 0 && Player.second.PlayerX < 20)
		{
			GotoXY(Player.second.PlayerX, Player.second.PlayerY);
			if (!Player.first.empty())
			{
				printf("%c", Player.first[Player.first.length() - 1]);
			}
		}
	}
}
