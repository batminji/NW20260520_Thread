#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32")

// Blocking Server
// Synchrous
// Multiplexing (polling)

int main()
{
	WSAData wsaData;
	int retval = 0;
	retval = WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(35000);

	connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	char Buffer[1024] = { 0, };

	while (true)
	{
		std::cin.getline(Buffer, sizeof(Buffer));

		int SendBytes = send(ServerSocket, Buffer, sizeof(Buffer), 0);
		if (SendBytes <= 0)
		{
			printf("send fail!\n");
			break;
		}
		else
		{
			int RecvBytes = recv(ServerSocket, Buffer, sizeof(Buffer), 0);
			if (RecvBytes <= 0)
			{
				printf("recv fail!\n");
				break;
			}
			else
			{
				printf("server send %s\n", Buffer);
			}
		}
	}

	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}