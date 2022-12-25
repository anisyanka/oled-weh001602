#ifndef OLED_WEH001602_H
#define OLED_WEH001602_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Any functions of the lib returns one of these values */
typedef enum
{
	OLED_WEH001602_OK,
	OLED_WEH001602_FAIL,
} oled_weh001602_ret_t;

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
} oled_weh001602_ll_t;

oled_weh001602_ret_t oled_init(oled_weh001602_ll_t *ll_funcs);

#ifdef __cplusplus
}
#endif

#endif  // OLED_WEH001602_H
