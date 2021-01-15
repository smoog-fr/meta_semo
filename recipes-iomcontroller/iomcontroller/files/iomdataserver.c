/****************** SERVER CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(){
	int welcomeSocket, newSocket;
	char buffer[1024];
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	int son_counter = 0;
	int val = 100;

	/*---- Create the socket. The three arguments are: ----*/
	/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
	welcomeSocket = socket(AF_INET, SOCK_STREAM, 0);

	/*---- Configure settings of the server address struct ----*/
	/* Address family = Internet */
	serverAddr.sin_family = AF_INET;
	/* Set port number, using htons function to use proper byte order */
	serverAddr.sin_port = htons(6000);
	/* Set IP address to localhost */
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/* Set all bits of the padding field to 0 */
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*---- Bind the address struct to the socket ----*/
	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	/*---- Listen on the socket, with 5 max connection requests queued ----*/
	if(listen(welcomeSocket,5)==0)
		printf("Listening\n");
	else
		printf("Error\n");

	/*---- Accept call creates a new socket for the incoming connection ----*/
	addr_size = sizeof serverStorage;

	while(1)
	{
		newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
		if (newSocket < 0) {
			perror("accept\n");
			perror(strerror (errno));
			return -1;
		}

		son_counter++;
		switch(fork()) {
		case 0: // Son
			printf("Forked son %u: Started\n", son_counter);
			int n;
			while(1) {
				printf("Forked son %u:Reading...\n", son_counter);
				n = recv(newSocket, buffer, 1024, 0);
				if (n == 0)
				{
					printf("Forked son %u:\n", son_counter);
					perror("recv(): Dead connection \n");
					break;
				}
				else if (n < 0)
				{
					printf("Forked son %u:\n", son_counter);
					perror("recv(): Error\n");
					perror(strerror (errno));
					break;
				}
				printf("Forked son %u: Data received: %s - %u\n", son_counter ,buffer, n);

				if (!strncmp(buffer, "get", strlen("get")))
				{
					printf("Forked son %u: get()\n", son_counter);
					val++;

					memset(buffer, '\0', sizeof(buffer));
					sprintf(buffer, "%u", val);
					printf("Forked son %u:Sending...\n", son_counter);

					n = send(newSocket, buffer, strnlen(buffer,50),0);
					if (n < 0)					{

						printf("Forked son %u:\n", son_counter);
						perror("send(): Error\n");
						perror(strerror (errno));
						break;
					}
					else
					{
						printf("Forked son %u: Response sent\n", son_counter);
					}
				}
			}

			close(newSocket);
			printf("Forked son %u: closed\n", son_counter);
			exit(EXIT_SUCCESS);
		case -1:
			printf("Forked son %u:\n", son_counter);
			perror("fork\n");
			return -1;
		default:	// Father
			// Close this since son has "newSocket" now
			close(newSocket);
		}
	}

	/*---- Send message to the socket of the incoming connection ----*/
	return 0;
}
