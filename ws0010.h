#ifndef OLED_WS0010_H
#define OLED_WS0010_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Any functions of the lib returns one of these values */
typedef enum
{
	WS0010_OK,
	WS0010_FAIL,
} ws0010_ret_t;

/* Low Level functions which must be implemented by user */
typedef struct
{
	/* us delay must be implemented fot strobe logic */
	void (*delay_us)(uint32_t us);

	/* control data pins */
	void (*set_bits_to_out_pins)(uint8_t value);

	/* control RS pin */
	void (*set_rs)(void);
	void (*reset_rs)(void);

	/* control RW pin */
	void (*set_rw)(void);
	void (*reset_rw)(void);

	/* control strobe pin */
	void (*set_e)(void);
	void (*reset_e)(void);
} ws0010_ll_t;

typedef struct
{
	/* data */
} oled;


ws0010_ret_t oled_init(ws0010_ll_t *ll_funcs);

#ifdef __cplusplus
}
#endif

#endif  // OLED_WS0010_H
