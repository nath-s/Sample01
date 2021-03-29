/*
	Server on port 5000
*/
#include<io.h>
#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library


int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET s, new_connection;
	struct sockaddr_in server, client;
	timeval tm;
	int c, SelectTiming;
	const char *message;
	char client_reply[2000];
	const char *heart_beat ,*close, *Exit;
	int byte_received, length, i, count = 0, ret;
	bool app_terminate = FALSE;

	int max_clients = 20, sd, activity, client_socket[20];
	int max_sd;

	//set of socket descriptor
	fd_set readfds;

	// Setup timeout variable
	struct timeval timeout;

	// assign the second and microsecond variables
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;

	//Message description
	heart_beat = "ALIVE\n";
	close = "Close\n";
	Exit = "Exit\n";

	//server welcome message to client
	message = "HI THERE";

	//initialise client socket to 0
	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;

	}

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket to listen for client connections
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket, error code : %d", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Socket created.\n");

	//Prepare the sockaddr_in structure for bind to listen for connection using port 5000
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(5000);

	//associate the information to Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		// Close the socket & clean up before exit
		closesocket(s);
		WSACleanup();

		return 1;
	}

	puts("Bind done");

	//Listen to client connections with a max number
	if (listen(s, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Listening failed with error code : %d", WSAGetLastError());
		// Close the socket & clean up before exit
		closesocket(s);
		WSACleanup();

		return 1;
	}

	//listening for incoming connection
	puts("Listening/waiting for incoming connections...");

	c = sizeof(struct sockaddr_in);
	//Accept all incoimg connections:

	while ((TRUE) && (app_terminate == FALSE))
	{
		//clear the socket set 
		FD_ZERO(&readfds);

		//add Listening master socket to set 
		FD_SET(s, &readfds);
		max_sd = s;

		//add child sockets to set : maximum number is 20
		for (i = 0; i < max_clients; i++)
		{
			//socket descriptor 
			sd = client_socket[i];

			//if valid socket descriptor then add to read list 
			if (sd > 0)
				FD_SET(sd, &readfds);

			//highest file descriptor number for the select function 
			if (sd > max_sd)
				max_sd = sd;
		}

		//wait for an activity on one of the sockets , timeout NULL ,so wait indefinitely 
		activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

		if ((activity < 0) && (errno != EINTR))
		{
			printf("select error");
		}

		//If something happened on the listening socket ,then its an incoming connection 
		if (FD_ISSET(s, &readfds))
		{
			//accept the connection
			if ((new_connection = accept(s,(struct sockaddr *)&client, &c)) < 0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands 
			printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_connection , inet_ntoa(client.sin_addr) , ntohs
				(client.sin_port));

			//send new connection Welcome message 
			if (send(new_connection, message, strlen(message), 0) != strlen(message))
			{
				perror("send");
			}

			puts("Welcome message sent successfully\n");

			//add new socket/connection to array of sockets 
			for (i = 0; i < max_clients; i++)
			{
				//if position is empty 
				if (client_socket[i] == 0)
				{
					client_socket[i] = new_connection;
					printf("Adding to list of sockets/connection as %d\n", i);

					break;
				}
			}
		}

		//else its some IO operation on some other socket
		for (i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds))
			{
				//Check if it was for closing , and also read the incoming message 

				byte_received = recv(new_connection, client_reply, sizeof(client_reply), 0);

				if ((byte_received > 0) && (count < 11))
				{
					printf("Server: recv() looks fine....\n");
					// Some info on the receiver side...
					getsockname(s, (SOCKADDR *)&server, (int *)sizeof(server));
					printf("Server: Receiving IP(s) used : %s\n", inet_ntoa(server.sin_addr));
					printf("Server: Receiving port used : %d\n", htons(server.sin_port));
					
					memset(&client, 0, sizeof(client));
					length = sizeof(client);
					// Some info on the sender side
					// Allocate the required resources

					getpeername(new_connection, (SOCKADDR *)&client, &length);
					printf("Server: Sending IP used : %s\n", inet_ntoa(client.sin_addr));
					printf("Server: Sending port used : %d\n", htons(client.sin_port));
					
					printf("Server: Bytes received : %d\n", byte_received);
					// Print what those bytes represent
					printf("Server: Those bytes are :");

					//Add a NULL terminating character to make it a proper string before printing
					client_reply[byte_received] = '\0';
					puts(client_reply);

					if (strcmp(client_reply, heart_beat) == 0)
					{
						printf("It is Heartbeat message\n");
						//if (shutdown(new_connection, SD_SEND) != 0)
							//printf("Server: Well, there is something wrong with the shutdown().The error code : %ld\n", WSAGetLastError());
						//break;
					}
					else if (strcmp(client_reply, close) == 0)
					{
						printf("It is close message so disconnect the socket\n");
						//Close the socket and mark as 0 in list for reuse 
						closesocket(sd);
						client_socket[i] = 0;
						//break;
					}
					else if (strcmp(client_reply, Exit) == 0)
					{
						printf("It is Exit message\n");
						printf("Stop Listenting to the port and terminating the app\n");
						//Close the socket and mark as 0 in list for reuse 
						//closesocket(sd);
						client_socket[i] = 0;
						app_terminate = TRUE;
						break;
					}
				}
				//10 message received
				else if ((byte_received > 0) && (count == 11))
				{
					printf("We have received 10 messages so disconnet\n");
					//Close the socket and mark as 0 in list for reuse 
					closesocket(sd);
					client_socket[i] = 0;
					//break;
				}

				// No data
				else if (byte_received == 0)
					printf("Server: Connection closed!\n");
				// Others
				else
				{
					printf("Server: recv() failed with error code : %d\n", WSAGetLastError());
					//Close the socket and mark as 0 in list for reuse 
					closesocket(sd);
					client_socket[i] = 0;
					//break;
				}
			} // if end
		} //for loop
	} //while (1)

//We are at this point because we got EXIT!!
	printf("Server: The listening socket is exiting...\n");
	// When all the data communication and listening finished, close the socket
	if (closesocket(sd) != 0)
		printf("Server: Cannot close \ListeningSocket\ socket.Error code : %ld\n", WSAGetLastError());
	else
		printf("Server: Closing \ListeningSocket\ socket...\n");

	// Finally, clean up all those WSA setup

	if (WSACleanup() != 0)
		printf("Server: WSACleanup() failed!Error code : %ld\n", WSAGetLastError());

	else
		printf("Server: WSACleanup() is OK...\n");

	return 0;
}