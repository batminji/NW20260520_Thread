#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <WinSock2.h>
#include <iostream>
#include <process.h>

#pragma comment(lib, "ws2_32")

using namespace rapidjson;

int main()
{
	// 1. Parse a JSON string into DOM.
	const char* json = R"(
{
	"project": "rapidjson",
	"stars": 10 ,
	"name" : "박민지",
	"result" : true
}
	)";

	Document d;
	d.Parse(json);

	// 2. Modify it by DOM.
	Value& s = d["stars"];
	s.SetInt(s.GetInt() + 1);

	std::cout << d["name"].GetString() << std::endl;

	d["name"] = "민지박";

	std::cout << d["result"].GetBool() << std::endl;

	// 3. Stringify the DOM
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	// Output {"project":"rapidjson","stars":11}
	std::cout << buffer.GetString() << std::endl;



	return 0;

}

char RecvBuffer[1024] = { 0, };
char SendBuffer[1024] = { 0, };

unsigned WINAPI RecvThread(void* Socket);
unsigned WINAPI SendThread(void* Socket);

int main2()
{
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

	// Blocking
	WaitForMultipleObjects(2, ThreadHandles, FALSE, INFINITE);

	while (true)
	{
		
	}

	closesocket(ServerSocket);

	CloseHandle(ThreadHandles[0]);
	CloseHandle(ThreadHandles[1]);

	WSACleanup();

	return 0;
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
		else
		{
			printf("server send %s\n", RecvBuffer);
		}
	}

	return 0;
}

unsigned __stdcall SendThread(void* Socket)
{
	SOCKET ServerSocket = *(SOCKET*)Socket;

	while (true)
	{
		std::cin.getline(SendBuffer, sizeof(SendBuffer));

		int SendBytes = send(ServerSocket, SendBuffer, sizeof(SendBuffer), 0);
		if (SendBytes <= 0)
		{
			printf("send fail!\n");
			break;
		}
	}

	return 0;
}
