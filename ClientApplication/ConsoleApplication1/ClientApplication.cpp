/*
	Create a TCP socket
*/

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library


// A sample of the select() return value

int recvTimeOutTCP(SOCKET socket, long sec, long usec)

{
	// Setup timeval variable
	struct timeval timeout;
	struct fd_set fds;

	// assign the second and microsecond variables
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;

	// Setup fd_set structure
	FD_ZERO(&fds);
	FD_SET(socket, &fds);

	// Possible return values:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read

	return select(0, &fds, 0, 0, &timeout);

}

int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	const char *message ;
	char server_reply[2000];
	int recv_size, SelectTiming;
	int i = 1, case_select = 0;
	

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Socket created.\n");

	//Prepare the sockaddr_in structure connection using port 5000
	server.sin_addr.s_addr = inet_addr("192.168.1.8"); //local ip address
	server.sin_family = AF_INET;
	server.sin_port = htons(5000);

	//Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		// Close the socket & clean up before exit
		closesocket(s);
		WSACleanup();
		return 1;
	}

	puts("Connected");

	//Receive a reply from the server
	if ((recv_size = recv(s, server_reply, 2000, 0)) == SOCKET_ERROR)
	{
		puts("recv failed");
	}

	puts("Reply received\n");

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	puts(server_reply);

	//Heartbeat message should be sent every 15 sec

	do {
		message = "ALIVE\n";
		if (send(s, message, strlen(message), 0) < 0)
		{
			puts("Send failed");
			return 1;
		}
		puts("Heartbeat Sent:ALIVE\n");
		case_select++;
		if (case_select < 5)
		{
			message = "Hello World\n";
			if (send(s, message, strlen(message), 0) < 0)
			{
				puts("Send failed");
				return 1;
			}
			puts("Message sent:Hello World\n");
		}
		else if (case_select == 6)
		{
			message = "Close\n";
			if (send(s, message, strlen(message), 0) < 0)
			{
				puts("Send failed");
				return 1;
			}
			puts(" Sent:Close\n");
		}
		else if (case_select >= 5)
		{
			message = "Exit\n";
			if (send(s, message, strlen(message), 0) < 0)
			{
				puts("Send failed");
				return 1;
			}
			puts(" Sent:Close\n");
		}
	} while (SelectTiming = recvTimeOutTCP(s, 5, 0) == 0);



#if 0
	//Receive a reply from the server
	if ((recv_size = recv(s, server_reply, 2000, 0)) == SOCKET_ERROR)
	{
		puts("recv failed");
	}

	puts("Reply received\n");

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	puts(server_reply);
#endif
	
	//closesocket(s);
	//WSACleanup();

	getchar();

	return 0;
}