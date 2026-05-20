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

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(35000);

	retval = bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

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
				}
				else
				{
					// Recv
					int RecvBytes = recv(ReadSockets.fd_array[i], Buffer, sizeof(Buffer), 0);
					if (RecvBytes <= 0)
					{
						SOCKADDR_IN CloseClientSockAddr;
						memset(&CloseClientSockAddr, 0, sizeof(CloseClientSockAddr));
						int CloseClientSockLength = sizeof(CloseClientSockAddr);

						getpeername(ReadSockets.fd_array[i], (SOCKADDR*)&CloseClientSockAddr, &CloseClientSockLength);
						printf("disconnect client %s\n", inet_ntoa(CloseClientSockAddr.sin_addr));
						closesocket(ReadSockets.fd_array[i]);
						FD_CLR(ReadSockets.fd_array[i], &ReadSockets);
					}
					else
					{
						SOCKADDR_IN ClientSockAddr;
						memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
						int ClientSockLength = sizeof(ClientSockAddr);

						getpeername(ReadSockets.fd_array[i], (SOCKADDR*)&ClientSockAddr, &ClientSockLength);
						printf("client %s send %s\n", inet_ntoa(ClientSockAddr.sin_addr), Buffer);

						// 모든 접속한 유저한테 전달
						for (int j = 0; i < (int)ReadSockets.fd_count; ++j)
						{
							if (ReadSockets.fd_array[i] != ListenSocket)
							{
								int SendBytes = send(ReadSockets.fd_array[i], Buffer, sizeof(Buffer), 0);
								if (SendBytes <= 0)
								{
									SOCKADDR_IN CloseClientSockAddr;
									memset(&CloseClientSockAddr, 0, sizeof(CloseClientSockAddr));
									int CloseClientSockLength = sizeof(CloseClientSockAddr);

									getpeername(ReadSockets.fd_array[i], (SOCKADDR*)&CloseClientSockAddr, &CloseClientSockLength);
									printf("send fail!\n");
									printf("disconnect client %s\n", inet_ntoa(CloseClientSockAddr.sin_addr));
									closesocket(ReadSockets.fd_array[i]);
									FD_CLR(ReadSockets.fd_array[i], &ReadSockets);
								}
							}
						}
					}
				}
			}
		}
	}

	closesocket(ListenSocket);

	return 0;
}