#ifndef SHIOMI_H
#define SHIOMI_H

#include <pthread.h>
#include <stdint.h>

typedef struct {
	uint16_t outputs;
	uint8_t leds;
} st_shiomi_data_t;

typedef struct {
	pthread_mutex_t m;
	pthread_cond_t c;
	st_shiomi_data_t data;
} st_shiomi_share_data_t;

extern int shiomi_create(void);
extern void shiomi_init(st_shiomi_share_data_t *data);
extern st_shiomi_share_data_t *shiomi_get(void);

#endif /* SHIOMI_H */
