#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include "shm_iom_in.h"
#include <stdlib.h>

#define NSEC_PER_SEC	1000000000UL

#define CYCLE_NS	500000000UL	// 250ms

static char * pip_name = "iomcontroller_in.fifo";

int main(){
	int clientSocket;
	char buffer[1024];
	int n;
	int err;
	uint16_t u16_temp;
	struct sched_param param;

	st_shiomi_share_data_t *sdi;

	struct sockaddr_in serverAddr;
	socklen_t addr_size;

	printf("I'm the new one (again x2)...\n");

	param.sched_priority = 80;
	n = sched_setscheduler(0, SCHED_FIFO, &param);

	printf("Scheduler return: \%u\r\n", n);

	/* Input shared memory init */
	err = shiomi_create();
	assert(!err);

	sdi = shiomi_get();
	assert(sdi);

	shiomi_init(sdi);

	/*---- Create the socket. The three arguments are: ----*/
	/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	/*---- Configure settings of the server address struct ----*/
	/* Address family = Internet */
	serverAddr.sin_family = AF_INET;
	/* Set port number, using htons function to use proper byte order */
	serverAddr.sin_port = htons(6000);
	/* Set IP address to localhost */
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/* Set all bits of the padding field to 0 */
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*---- Connect the socket to the server using the address struct ----*/
	addr_size = sizeof serverAddr;

	err = connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
	if (err)
	{
		perror(strerror (errno));
		return -1;
	}
	/*---- Read the message from the server into the buffer ----*/
	//recv(clientSocket, buffer, 1024, 0);

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	while(1)
	{
		// Ask server for info
		memset(buffer, '\0', sizeof(buffer));
		strcpy(buffer, "get");
		send(clientSocket, buffer, strnlen(buffer,50),0);

		// Read info from server
		printf("Moving to receiving...\n");
		memset(buffer, '\0', sizeof(buffer));
		n = recv(clientSocket, buffer, 1024, 0);
		printf("Received something...\n");
		if (n == 0)
		{
			perror("recv(): Dead connection \n");
			perror(strerror (errno));
			break;
		}
		else if (n < 0)
		{
			perror("recv(): Error\n");
			perror(strerror (errno));
			break;
		}
		printf("get() has returned: %s\n", buffer);
		u16_temp = strtol(buffer, (char **)NULL, 10);

		/* Now synchronize shared memory */
		pthread_mutex_lock(&sdi->m);
		printf("Update\r\n");
		sdi->data.outputs = u16_temp;

		pthread_cond_broadcast(&sdi->c);
		pthread_mutex_unlock(&sdi->m);

		/* Some delay */
		

		ts.tv_nsec += CYCLE_NS;
		while (ts.tv_nsec >= NSEC_PER_SEC) {
			ts.tv_sec++;
			ts.tv_nsec -= NSEC_PER_SEC;
		}
		if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
				&ts, NULL) != 0) {
			break;
		}
	}
	/*---- Print the received message ----*/
	//printf("Data received: %s",buffer);

	close(clientSocket);
	printf("client suicide\n");

	return 0;
}
