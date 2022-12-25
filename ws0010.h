#ifndef OLED_WS0010_H
#define OLED_WS0010_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Remember when power is turned ON,
 * WS0010 is initialized automatically by an internal reset circuit
 * to this state:
 * 1. Display clear
 *
 * 2. Function set:
 *     DL="1": 8-bit interface data
 *     N="0": 1-line display
 *     F="0": 5 x 8 dot character font
 * 3. Power turn off
 *     PWR=”0”
 * 4. Display on/off control: D="0": Display off C="0": Cursor off B="0": Blinking off
 * 5. Entry mode set
 *     I/D="1": Increment by 1
 *     S="0": No shift
 * 6. Cursor/Display shift/Mode / Pwr
 *     S/C=”0”, R/L=”1”: Shifts cursor position to the right
 *     G/C=”0”: Character mode
 *     Pwr=”1”: Internal DC-DC power on
 **/

/* Any functions of the lib returns one of these values */
typedef enum
{
	WS0010_OK,
	WS0010_FAIL,
} ws0010_ret_t;

/* Character font size */
typedef enum
{
	WS0010_5x8DOTS,
	WS0010_5x10DOTS,
} ws0010_dots_t;

/* Interface data len */
typedef enum
{
	WS0010_8BITS,
	WS0010_4BITS,
} ws0010_bits_t;

typedef enum
{
	ENG_JAPAN,
	WESTERN_EUROPEAN_1,
	WESTERN_EUROPEAN_2,
	ENG_RUS,
} ws0010_alph_t;

/* Low Level functions which must be implemented by user */
typedef struct
{
	/* us delay must be implemented fot strobe logic */
	void (*delay_us)(uint32_t us);

	/*
	 * Control data pins.
	 * Implement these func depending on display mode:
	 *     8-bits - set all 8 bits to pins
	 *     4-bits - set least significant bits (0-3) to pins
	 **/
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
	/* common count of lines in your display */
	int line_count;

	/* displayed symbols per one line */
	int symbols_per_line;

	/* take in account symbols not displayed on screen (for shifting) */
	int max_symbols_per_line;

	/* dots per characters */
	ws0010_dots_t font_size;

	/* interface length */
	ws0010_bits_t interface_bits;

	/* table of characters */
	ws0010_alph_t alphabet;

	/* hardware dependent functions */
	ws0010_ll_t *ll;

	/* internally used variables */
	uint8_t _entrymode_state;
	uint8_t _display_control_state;
	uint8_t _function_set;
} ws0010_dev_t;

/* Call it before others */
ws0010_ret_t ws0010_init(ws0010_dev_t *dev);

/* This is used to clear the Display Write Space 20H in all DDRAM Addresses. */
ws0010_ret_t ws0010_clear(ws0010_dev_t *dev);

/*
 * Set the DDRAM Address 0 into the Address Counter
 * and revert the display to its original status (if the display has been shifted).
 **/
ws0010_ret_t ws0010_home(ws0010_dev_t *dev);

/* control display state */
ws0010_ret_t ws0010_display_on(ws0010_dev_t *dev);
ws0010_ret_t ws0010_display_off(ws0010_dev_t *dev);
ws0010_ret_t ws0010_cursor_on(ws0010_dev_t *dev);
ws0010_ret_t ws0010_cursor_off(ws0010_dev_t *dev);
ws0010_ret_t ws0010_blink_on(ws0010_dev_t *dev);
ws0010_ret_t ws0010_blink_off(ws0010_dev_t *dev);

#ifdef __cplusplus
}
#endif

#endif  // OLED_WS0010_H
