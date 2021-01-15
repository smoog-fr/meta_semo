#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include "shm_iom_in.h"
#include "shm_iom_out.h"

#define NSEC_PER_SEC	1000000000UL

#define CYCLE_NS	50000000UL	// 50ms
#define SLEEP_NS	1000000UL	// 2ms

static char *portname = "/dev/ttymxc4";
//static char *portname = "/dev/ttyUSB0";

#define RX_LEN 50
#define TX_LEN 50

//static uint8_t counter;
static char tx_buf[TX_LEN];
static char rx_buf[RX_LEN];

static int set_interface_attribs (int fd, int speed, int parity);
static void set_blocking (int fd, int should_block);

int main(void)
{
	st_shiomi_share_data_t *sdi;
	st_shiomo_share_data_t *sdo;
	st_shiomi_data_t in_data;

	struct timespec ts;
	int err;
	int n;

	struct sched_param param;

	//setup_rt(0xf);

	param.sched_priority = 80;
	n = sched_setscheduler(0, SCHED_FIFO, &param);

	printf("Scheduler return: \%u\r\n", n);

	/* Input shared memory init */
	sdi = shiomi_get();
	assert(sdi);

	/* Output shared memory init */
	err = shiomo_create();
	assert(!err);

	sdo = shiomo_get();
	assert(sdo);

	shiomo_init(sdo);

	/* Serial port */
	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
	{
		perror(strerror (errno));
		return 0;
	}

	set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	do {
		n = read (fd, rx_buf, sizeof rx_buf);
	} while(n > 0);

	/* Init done, running now ----------------------------------------------- */
	//pthread_mutex_lock(&sdi->m);

	printf("Enter while()...\r\n");
	while (1) {
		/* Wait for new entry ----------------------------------------------- */
		pthread_mutex_lock(&sdi->m);
		err = pthread_cond_wait(&sdi->c, &sdi->m);
		if (err) {
			perror(strerror (err));
			pthread_mutex_unlock(&sdi->m);
			break;
		}
		printf("---------------------------------------------\n");
		//printf("   Loop\r\n");

		/* Copy data */
		in_data = sdi->data;	// to be checked

		/* Release mutex */
		pthread_mutex_unlock(&sdi->m);

		/* Prepare message -------------------------------------------------- */
		n = snprintf(tx_buf, sizeof(tx_buf), "%u", in_data.outputs);
		//tx_buf[n++] = '\r';
		tx_buf[n++] = '\n';
		tx_buf[n++] = '\0';

		printf("Send now: %s\n", tx_buf);

		/* --------------------------- Send message ------------------------- */
		write(fd, tx_buf, strnlen(tx_buf, TX_LEN));
		// TODO: replace with order to modbus master

		/* Wait for feedback ------------------------------------------------ */
		memset(rx_buf, 0, RX_LEN);

		/* Some delay */
		clock_gettime(CLOCK_MONOTONIC, &ts);

		ts.tv_nsec += SLEEP_NS;
		while (ts.tv_nsec >= NSEC_PER_SEC) {
			ts.tv_sec++;
			ts.tv_nsec -= NSEC_PER_SEC;
		}
		if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
				&ts, NULL) != 0) {
			break;
		}

		n = read (fd, rx_buf, sizeof rx_buf);
		if (n != 0)	{
			printf("Read from UART");printf(rx_buf);
		} else {
			printf("No return"); printf("\n");
		}

		/* Now synchronize data out ----------------------------------------- */
		pthread_mutex_lock(&sdo->m);

		/* real-time work */
		// TODO: hereunder just for test
		//memcpy(sdo->data.firmware_ver, rx_buf, strlen(rx_buf));

		pthread_cond_broadcast(&sdo->c);
		pthread_mutex_unlock(&sdo->m);

		//printf("   EO Loop\r\n");
	}

	return 0;
}


/* source : http://stackoverflow.com/a/6947758 */
int set_interface_attribs (int fd, int speed, int parity) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0) {
		perror ("error from tcgetattr");
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		perror ("error from tcsetattr");
		return -1;
	}
	return 0;
}

void set_blocking (int fd, int should_block) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		perror ("error from tggetattr");
		return;
	}
	//http://unixwiz.net/techtips/termios-vmin-vtime.html
	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 0;            // 5 - 0.5 seconds read timeout
	// TODO: VTIME should not be 0 (in normal operation should return)

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		perror ("error setting term attributes");
}
