#include "ws0010.h"
#include <stdint.h>

#define FIRST_LINE_HEAD_ADDR	((uint8_t)0x00)
#define SECOND_LINE_HEAD_ADDR	((uint8_t)0x40)

#define CHECK_BUSY_ATTEMPTS		200
#define DELAY_BETWEEN_CHECKS_US 5000

#define DISPLAY_COMMAND_MODE	0
#define DISPLAY_DATA_MODE		1

#define CLEAR_DISPLAY_CMD		((uint8_t)0x01)
#define RETURN_HOME_CMD			((uint8_t)0x02)

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
static int is_busy(ws0010_dev_t *dev)
{
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

	return 0;
}

static int is_args_ok(ws0010_dev_t *dev)
{
	if (!dev || !dev->ll->delay_us || !dev->ll->reset_e ||
		!dev->ll->reset_rs || !dev->ll->reset_rw ||
		!dev->ll->set_bits_to_out_pins || !dev->ll->set_e ||
		!dev->ll->set_rs || !dev->ll->set_rw) {
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

	if (dev->bits == WS0010_8BITS) {
		dev->ll->set_bits_to_out_pins(byte);
		pulse_strobe(dev);
	} else if (dev->bits == WS0010_4BITS) {
		dev->ll->set_bits_to_out_pins(byte >> 4);
		pulse_strobe(dev);

		dev->ll->set_bits_to_out_pins(byte);
		pulse_strobe(dev);
	} else {
		return WS0010_FAIL;
	}

	return WS0010_OK;
}

ws0010_ret_t ws0010_init(ws0010_dev_t *dev)
{
	if (!is_args_ok(dev)) {
		return WS0010_FAIL;
	}

	/* wait for power stabilization */
	dev->ll->delay_us(50000);

	// function set

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

ws0010_ret_t ws0010_function_set(ws0010_dev_t *dev)
{
	return WS0010_OK;
}