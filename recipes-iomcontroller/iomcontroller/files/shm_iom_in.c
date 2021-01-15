#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "shm_iom_in.h"

#define SHARE_NAME "/shm_iom_in.shm"

/* open and return a pointer to shared data */
st_shiomi_share_data_t *shiomi_get(void)
{
	st_shiomi_share_data_t *data;
	int fd;

	fd = shm_open(SHARE_NAME, O_RDWR, 0);
	assert(fd != -1);

	data = mmap(NULL, sizeof(st_shiomi_share_data_t), PROT_READ | PROT_WRITE,
		    MAP_SHARED, fd, 0);
	assert(data != MAP_FAILED);

	close(fd);

	return data;
}

/* initialize shared data */
void shiomi_init(st_shiomi_share_data_t *data)
{
	pthread_mutexattr_t mattr;
	pthread_condattr_t cattr;

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&data->m, &mattr);

	pthread_condattr_init(&cattr);
	pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&data->c, &cattr);
}

/* create a zero'd out file, size of 2 pages */
int shiomi_create(void)
{
	int ret;
	int fd;

	fd = shm_open(SHARE_NAME, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	assert(fd != -1);

	ret = ftruncate(fd, sizeof(st_shiomi_share_data_t));

	close(fd);

	return ret;
}
