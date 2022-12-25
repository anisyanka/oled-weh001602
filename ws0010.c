#include "ws0010.h"
#include <stdint.h>
#include <stddef.h>

#define FIRST_LINE_HEAD_ADDR ((uint8_t)0x00)
#define SECOND_LINE_HEAD_ADDR ((uint8_t)0x40)
#define LAST_AVAILABLE_ADDR ((uint8_t)(0x7F))

#define CHECK_BUSY_ATTEMPTS 200
#define DELAY_BETWEEN_CHECKS_US 5000

#define DISPLAY_COMMAND_MODE 0
#define DISPLAY_DATA_MODE 1

#define CLEAR_DISPLAY_CMD ((uint8_t)0x01)
#define RETURN_HOME_CMD ((uint8_t)0x02)
#define DISPLAY_ENTMODESET_CMD ((uint8_t)0x04)
#define DISPLAY_CONTROL_CMD ((uint8_t)0x08)
#define DISPLAY_FUNCTIONSET_CMD ((uint8_t)0x20)
#define DISPLAY_SET_DDRAM_ADDR_CMD ((uint8_t)0x80)

/* display state defines */
#define DISPLAY_ON_OFF_POS 2
#define CURSOR_ON_OFF_POS 1
#define BLINKING_ON_OFF_POS 0

/* entry mode set defines */
#define INC_DEC_POS 1
#define SHIFT_POS 0

/* function set defines */
#define DATA_LEN_POS 4
#define NUMBER_OF_LINE_POS 3
#define FONT_POS 2

static void set_pins_to_write_cmd(ws0010_dev_t *dev)
{
	dev->ll->reset_rs();
	dev->ll->reset_rw();
}

static void set_pins_to_write_data(ws0010_dev_t *dev)
{
	dev->ll->set_rs();
	dev->ll->reset_rw();
}

/* wait until busy or return fail code: 1 */
// static int is_busy(ws0010_dev_t *dev)
// {
	// int check_busy_cycles = 0;

	// read DB7
	// while (DB7 == 1)
	// {
	// 	++check_busy_cycles;
	// 	if (check_busy_cycles >= CHECK_BUSY_ATTEMPTS) {
	// 		return 1;
	// 	}
	// 	dev->ll->delay_us(DELAY_BETWEEN_CHECKS_US);
	// 	read DB7	
	// }

// 	return 0;
// }

static int is_args_ok(ws0010_dev_t *dev)
{
	if (!dev || !dev->ll->delay_us || !dev->ll->reset_e ||
		!dev->ll->reset_rs || !dev->ll->reset_rw ||
		!dev->ll->set_bits_to_out_pins || !dev->ll->set_e ||
		!dev->ll->set_rs || !dev->ll->set_rw) {
		return 0;	
	}

	if (dev->line_count > 2 || dev->line_count <= 0) {
		return 0;
	}

	return 1;
}

static void pulse_strobe(ws0010_dev_t *dev)
{
	dev->ll->reset_e();
	dev->ll->delay_us(1);

	dev->ll->set_e();
	dev->ll->delay_us(1);

	dev->ll->reset_e();
	dev->ll->delay_us(100);
}

static ws0010_ret_t write(ws0010_dev_t *dev, uint8_t byte, int mode)
{
	if (mode == DISPLAY_COMMAND_MODE) {
		set_pins_to_write_cmd(dev);
	} else if (mode == DISPLAY_DATA_MODE) {
		set_pins_to_write_data(dev);
	} else {
		return WS0010_FAIL;
	}

	if (dev->interface_bits == WS0010_8_BITS) {
		dev->ll->set_bits_to_out_pins(byte);
		pulse_strobe(dev);
	} else if (dev->interface_bits == WS0010_4_BITS) {
		dev->ll->set_bits_to_out_pins(byte >> 4);
		pulse_strobe(dev);

		dev->ll->set_bits_to_out_pins(byte);
		pulse_strobe(dev);
	} else {
		return WS0010_FAIL;
	}

	return WS0010_OK;
}

static ws0010_ret_t ws0010_entry_mode_set(ws0010_dev_t *dev,
                                          int inc_bit,
                                          int shift_bit)
{
	dev->_entrymode_state = (uint8_t)(inc_bit << INC_DEC_POS) | \
                            (uint8_t)(shift_bit);
	ws0010_ret_t ret = write(dev,
                             DISPLAY_ENTMODESET_CMD | dev->_entrymode_state,
                             DISPLAY_COMMAND_MODE);
	return ret;
}

static ws0010_ret_t ws0010_function_set(ws0010_dev_t *dev,
                                        ws0010_bits_t dl,
                                        uint8_t n,
                                        ws0010_dots_t font_size,
                                        ws0010_alph_t alphabet)
{
	uint8_t _dl = 0;
	uint8_t _n = 0;
	uint8_t _font_size = 0;
	uint8_t _alphabet = 0;

	/* data bus length */
	if (dl == WS0010_8_BITS) {
		_dl = (1 << DATA_LEN_POS);
	} else if (dl == WS0010_4_BITS) {
		_dl = 0;
	} else {
		return WS0010_FAIL;
	}

	/* lines count */
	if (n == 2) {
		_n = (1 << NUMBER_OF_LINE_POS);
	} else if (n == 1) {
		_n = 0;
	} else {
		return WS0010_FAIL;
	}

	/* font size */
	if (font_size == WS0010_5x10_DOTS) {
		_font_size = (1 << FONT_POS);
	} else if (font_size == WS0010_5x8_DOTS) {
		_font_size = 0;
	} else {
		return WS0010_FAIL;
	}

	/* set alphabet */
	if (alphabet == ENG_JAPAN) {
		_alphabet = 0;
	} else if (alphabet == WESTERN_EUROPEAN_1) {
		_alphabet = 1;
	} else if (alphabet == ENG_RUS) {
		_alphabet = 2;
	} else if (alphabet == WESTERN_EUROPEAN_2) {
		_alphabet = 3;
	} else {
		return WS0010_FAIL;
	}

	dev->_function_set = _dl | _n | _font_size | _alphabet;
	ws0010_ret_t ret = write(dev,
                             DISPLAY_FUNCTIONSET_CMD | dev->_function_set,
                             DISPLAY_COMMAND_MODE);
	return ret;
}

ws0010_ret_t ws0010_init(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = 0;

	if (!is_args_ok(dev)) {
		return WS0010_FAIL;
	}

	/* wait for power stabilization */
	dev->ll->delay_us(50000);

	/* function set */
	ret = ws0010_function_set(dev, dev->interface_bits, dev->line_count,
                              dev->font_size, dev->alphabet);
	if (ret == WS0010_FAIL) {
		return WS0010_FAIL;
	}
	dev->ll->delay_us(4500);

	/* Display on/off control */
	dev->_display_control_state = (1 << DISPLAY_ON_OFF_POS) | \
                                  (0 << CURSOR_ON_OFF_POS) | \
                                  (0 << BLINKING_ON_OFF_POS);
	ret = write(dev, DISPLAY_CONTROL_CMD | dev->_display_control_state,
                DISPLAY_COMMAND_MODE);
	if (ret == WS0010_FAIL) {
		return WS0010_FAIL;
	}
	dev->ll->delay_us(4500);

	/* clear */
	ret = ws0010_clear(dev);
	if (ret == WS0010_FAIL) {
		return WS0010_FAIL;
	}

	/* entry mode set */
	ret = ws0010_entry_mode_set(dev, 1, 0);
	if (ret == WS0010_FAIL) {
		return WS0010_FAIL;
	}

	return WS0010_OK;
}

ws0010_ret_t ws0010_clear(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = write(dev, CLEAR_DISPLAY_CMD, DISPLAY_COMMAND_MODE);
	dev->ll->delay_us(7000);

	return ret;
}

ws0010_ret_t ws0010_home(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = write(dev, RETURN_HOME_CMD, DISPLAY_COMMAND_MODE);
	dev->ll->delay_us(2000);

	return ret;
}

ws0010_ret_t ws0010_display_on(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = WS0010_FAIL;

	dev->_display_control_state |= (1 << DISPLAY_ON_OFF_POS);
	ret = write(dev, DISPLAY_CONTROL_CMD | dev->_display_control_state,
                DISPLAY_COMMAND_MODE);

	return ret;
}

ws0010_ret_t ws0010_display_off(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = WS0010_FAIL;

	dev->_display_control_state &= ~(1 << DISPLAY_ON_OFF_POS);
	ret = write(dev, DISPLAY_CONTROL_CMD | dev->_display_control_state,
                DISPLAY_COMMAND_MODE);

	return ret;
}

ws0010_ret_t ws0010_cursor_on(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = WS0010_FAIL;

	dev->_display_control_state |= (1 << CURSOR_ON_OFF_POS);
	ret = write(dev, DISPLAY_CONTROL_CMD | dev->_display_control_state,
                DISPLAY_COMMAND_MODE);

	return ret;
}

ws0010_ret_t ws0010_cursor_off(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = WS0010_FAIL;

	dev->_display_control_state &= ~(1 << CURSOR_ON_OFF_POS);
	ret = write(dev, DISPLAY_CONTROL_CMD | dev->_display_control_state,
                DISPLAY_COMMAND_MODE);

	return ret;
}

ws0010_ret_t ws0010_blink_on(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = WS0010_FAIL;

	dev->_display_control_state |= (1 << BLINKING_ON_OFF_POS);
	ret = write(dev, DISPLAY_CONTROL_CMD | dev->_display_control_state,
                DISPLAY_COMMAND_MODE);

	return ret;
}

ws0010_ret_t ws0010_blink_off(ws0010_dev_t *dev)
{
	ws0010_ret_t ret = WS0010_FAIL;

	dev->_display_control_state &= ~(1 << BLINKING_ON_OFF_POS);
	ret = write(dev, DISPLAY_CONTROL_CMD | dev->_display_control_state,
                DISPLAY_COMMAND_MODE);

	return ret;
}

ws0010_ret_t ws0010_set_ddram_addr(ws0010_dev_t *dev, uint8_t addr)
{
	ws0010_ret_t ret = WS0010_FAIL;

	if (addr > LAST_AVAILABLE_ADDR) {
		return WS0010_FAIL;
	}

	ret = write(dev, DISPLAY_SET_DDRAM_ADDR_CMD | addr, DISPLAY_COMMAND_MODE);

	return ret;
}

ws0010_ret_t ws0010_print(ws0010_dev_t *dev, char *str, size_t len)
{
	size_t i = 0;
	ws0010_ret_t ret = WS0010_FAIL;

	for (i = 0; i < len; ++i) {
		ret |= write(dev, str[i], DISPLAY_DATA_MODE);
	}

	return ret;
}
