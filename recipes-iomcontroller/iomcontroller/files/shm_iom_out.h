#ifndef SHIOMO_H
#define SHIOMO_H

#include <pthread.h>
#include <stdint.h>

typedef struct {
	uint16_t inputs;
	char firmware_ver[50];
	uint8_t status;
} st_shiomo_data_t;

typedef struct {
	pthread_mutex_t m;
	pthread_cond_t c;
	st_shiomo_data_t data;
} st_shiomo_share_data_t;

extern int shiomo_create(void);
extern void shiomo_init(st_shiomo_share_data_t *data);
extern st_shiomo_share_data_t *shiomo_get(void);

#endif /* SHIOMO_H */
