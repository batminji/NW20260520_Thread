#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ChatPacket.h"
#include "CS_PlayerDir.h"
#include "SC_PlayerPos.h"

#include <WinSock2.h>
#include <iostream>
#include <process.h>
#include "json.hpp"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "NetCommon")

// Blocking Server
// Synchrous
// Multiplexing (polling)

struct PlayerInfo 
{
	std::string UserID = "";
	int PlayerX = 0;
	int PlayerY = 0;
};

std::map<SOCKET, PlayerInfo> ClientPlayers;

unsigned WINAPI RenderThread(void* Socket);

int main()
{
	WSAData wsaData;
	int retval = 0;
	retval = WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(35000);

	retval = ::bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, SOMAXCONN);

	// Blocking, synchronous(Time Out) - 성능은 떨어짐, 호환성(Windows, Linux, Mac), 쉬워서 사용
	TIMEVAL TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 500000;

	fd_set ReadSockets;
	fd_set CopyReadSockets;
	FD_ZERO(&ReadSockets);
	FD_SET(ListenSocket, &ReadSockets);

	char Buffer[1024] = { 0, };

	// Thread
	HANDLE ThreadHandles[1];

	ThreadHandles[0] = (HANDLE)_beginthreadex(0, 0, RenderThread, nullptr, 0, 0);

	while (true)
	{
		CopyReadSockets = ReadSockets;

		// Blocking (TimeOut 마다) (TimeOut이 0이면 무한히 기다림)
		int ChangeCount = select(0, &CopyReadSockets, 0, 0, &TimeOut);
		if (ChangeCount <= 0)
		{
			continue;
		}

		for (int i = 0; i < ReadSockets.fd_count; ++i)
		{
			if (FD_ISSET(ReadSockets.fd_array[i], &CopyReadSockets))
			{
				if (ReadSockets.fd_array[i] == ListenSocket)
				{
					// 신규 접속
					SOCKADDR_IN ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClientSockLength = sizeof(ClientSockAddr);

					// Blocking
					SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockLength);

					printf("connect client %s\n", inet_ntoa(ClientSockAddr.sin_addr));

					FD_SET(ClientSocket, &ReadSockets);

					PlayerInfo NewPlayer;
					NewPlayer.PlayerX = 0;
					NewPlayer.PlayerY = 0;
					ClientPlayers[ClientSocket] = NewPlayer;
				}
				else
				{
					// 기존 클라
					SOCKET CurrentClientSocket = ReadSockets.fd_array[i];

					// Recv
					memset(Buffer, 0, sizeof(Buffer));
					int RecvBytes = recv(CurrentClientSocket, Buffer, sizeof(Buffer), 0);

					// 접속 종료
					if (RecvBytes <= 0)
					{
						SOCKADDR_IN CloseClientSockAddr;
						memset(&CloseClientSockAddr, 0, sizeof(CloseClientSockAddr));
						int CloseClientSockLength = sizeof(CloseClientSockAddr);

						getpeername(CurrentClientSocket, (SOCKADDR*)&CloseClientSockAddr, &CloseClientSockLength);
						printf("disconnect client %s\n", inet_ntoa(CloseClientSockAddr.sin_addr));
						ClientPlayers.erase(CurrentClientSocket); // map에서 제거
						closesocket(CurrentClientSocket);
						FD_CLR(CurrentClientSocket, &ReadSockets);
					}
					else
					{
						Buffer[RecvBytes] = '\0';

						CS_PlayerDir RecvPacket;
						RecvPacket.Parse(Buffer);

						ClientPlayers[CurrentClientSocket].UserID = RecvPacket.UserID;

						if (RecvPacket.Dir == 'W' || RecvPacket.Dir == 'w')
						{
							ClientPlayers[CurrentClientSocket].PlayerY -= 1;
						}
						else if (RecvPacket.Dir == 'S' || RecvPacket.Dir == 's')
						{
							ClientPlayers[CurrentClientSocket].PlayerY += 1;
						}
						else if (RecvPacket.Dir == 'A' || RecvPacket.Dir == 'a')
						{
							ClientPlayers[CurrentClientSocket].PlayerX -= 1;
						}
						else if (RecvPacket.Dir == 'D' || RecvPacket.Dir == 'd')
						{
							ClientPlayers[CurrentClientSocket].PlayerX += 1;
						}

						// std::cout << RecvPacket.UserID << " X: " << ClientPlayers[CurrentClientSocket].PlayerX << " Y: " << ClientPlayers[CurrentClientSocket].PlayerY << std::endl;

						SC_PlayerPos SendPacket;
						for (const auto& Player : ClientPlayers)
						{
							PlayerData Data;
							Data.UserID = Player.second.UserID;
							Data.PlayerX = Player.second.PlayerX;
							Data.PlayerY = Player.second.PlayerY;
							SendPacket.Players.push_back(Data);
						}

						std::string JSONOutput = SendPacket.ToString();

						// 브로드 캐스트
						for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
						{
							SOCKET CurrentClientSocket = ReadSockets.fd_array[j];

							if (CurrentClientSocket != ListenSocket)
							{
								int SentBytes = send(CurrentClientSocket, JSONOutput.c_str(), (int)JSONOutput.length(), 0);

								if (SentBytes <= 0)
								{
									SOCKADDR_IN ClosedSockAddr;
									memset(&ClosedSockAddr, 0, sizeof(ClosedSockAddr));
									int ClosedSockAddrLength = sizeof(ClosedSockAddr);

									getpeername(CurrentClientSocket, (SOCKADDR*)&ClosedSockAddr, &ClosedSockAddrLength);
									printf("send fail!\n");
									printf("disconnect client %s\n", inet_ntoa(ClosedSockAddr.sin_addr));

									ClientPlayers.erase(CurrentClientSocket);
									FD_CLR(CurrentClientSocket, &ReadSockets);
									closesocket(CurrentClientSocket);
								}
							}
						}
					}
				}
			}
		}
	}

	closesocket(ListenSocket);

	CloseHandle(ThreadHandles[0]);

	WSACleanup();

	return 0;
}

unsigned WINAPI RenderThread(void* Socket)
{
	while (true)
	{
		system("cls");
		for (const auto& Player : ClientPlayers)
		{
			if (Player.second.PlayerY >= 0 && Player.second.PlayerY < 20
				&& Player.second.PlayerX >= 0 && Player.second.PlayerX < 20)
			{
				GotoXY(Player.second.PlayerX, Player.second.PlayerY);
				if (!Player.second.UserID.empty())
				{
					printf("%c", Player.second.UserID[Player.second.UserID.length() - 1]);
				}
			}
		}
	}

	return 0;
}