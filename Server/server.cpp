#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ChatPacket.h"
#include "CS_PlayerDir.h"
#include "SC_PlayerPos.h"

#include <WinSock2.h>
#include <iostream>
#include <map>
#include <string>
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
					SOCKET CurrentClientSocket = ReadSockets.fd_array[i];

					// Recv
					memset(Buffer, 0, sizeof(Buffer));
					int RecvBytes = recv(CurrentClientSocket, Buffer, sizeof(Buffer), 0);
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
						// SOCKADDR_IN ClientSockAddr;
						// memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
						// int ClientSockLength = sizeof(ClientSockAddr);
						// 
						// getpeername(ReadSockets.fd_array[i], (SOCKADDR*)&ClientSockAddr, &ClientSockLength);
						// printf("client %s send %s\n", inet_ntoa(ClientSockAddr.sin_addr), Buffer);
						// 
						// // 모든 접속한 유저한테 전달
						// for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
						// {
						// 	if (ReadSockets.fd_array[j] != ListenSocket)
						// 	{
						// 		int SentBytes = send(ReadSockets.fd_array[j], Buffer, (int)strlen(Buffer), 0);
						// 		if (SentBytes <= 0)
						// 		{
						// 			SOCKADDR_IN ClosedSockAddr;
						// 			memset(&ClosedSockAddr, 0, sizeof(ClosedSockAddr));
						// 			int ClosedSockAddrLength = sizeof(ClosedSockAddr);
						// 
						// 			SOCKET ClosedSocket = ReadSockets.fd_array[j];
						// 			getpeername(ClosedSocket, (SOCKADDR*)&ClosedSockAddr, &ClosedSockAddrLength);
						// 			printf("send fail!\n");
						// 			printf("disconnect client %s\n", inet_ntoa(ClosedSockAddr.sin_addr));
						// 			FD_CLR(ReadSockets.fd_array[j], &ReadSockets);
						// 			closesocket(ClosedSocket);
						// 		}
						// 	}
						// }
						Buffer[RecvBytes] = '\0';

						CS_PlayerDir RecvPacket;
						RecvPacket.Parse(Buffer);

						ClientPlayers[CurrentClientSocket].UserID = RecvPacket.UserID;

						if (RecvPacket.Dir == 'W' || RecvPacket.Dir == 'w') ClientPlayers[CurrentClientSocket].PlayerY -= 1;
						else if (RecvPacket.Dir == 'S' || RecvPacket.Dir == 's') ClientPlayers[CurrentClientSocket].PlayerY += 1;
						else if (RecvPacket.Dir == 'A' || RecvPacket.Dir == 'a') ClientPlayers[CurrentClientSocket].PlayerX -= 1;
						else if (RecvPacket.Dir == 'D' || RecvPacket.Dir == 'd') ClientPlayers[CurrentClientSocket].PlayerX += 1;

						std::cout << "[Update] " << RecvPacket.UserID
							<< " -> X: " << ClientPlayers[CurrentClientSocket].PlayerX
							<< ", Y: " << ClientPlayers[CurrentClientSocket].PlayerY << std::endl;

						SC_PlayerPos SendPacket;
						SendPacket.UserID = ClientPlayers[CurrentClientSocket].UserID;
						SendPacket.PlayerX = ClientPlayers[CurrentClientSocket].PlayerX;
						SendPacket.PlayerY = ClientPlayers[CurrentClientSocket].PlayerY;

						std::string JSONOutput = SendPacket.ToString();

						for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
						{
							SOCKET TargetSocket = ReadSockets.fd_array[j];

							if (TargetSocket != ListenSocket)
							{
								int SentBytes = send(TargetSocket, JSONOutput.c_str(), (int)JSONOutput.length(), 0);

								if (SentBytes <= 0)
								{
									SOCKADDR_IN ClosedSockAddr;
									memset(&ClosedSockAddr, 0, sizeof(ClosedSockAddr));
									int ClosedSockAddrLength = sizeof(ClosedSockAddr);

									getpeername(TargetSocket, (SOCKADDR*)&ClosedSockAddr, &ClosedSockAddrLength);
									printf("send fail!\n");
									printf("disconnect client %s\n", inet_ntoa(ClosedSockAddr.sin_addr));

									ClientPlayers.erase(TargetSocket);
									FD_CLR(TargetSocket, &ReadSockets);
									closesocket(TargetSocket);
								}
							}
						}
					}
				}
			}
		}
	}

	closesocket(ListenSocket);

	WSACleanup();

	return 0;
}