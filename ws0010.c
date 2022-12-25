#include "ws0010.h"
#include <stdint.h>

static void write_cmd(uint8_t cmd)
{
	return;
}

static void write_data(uint8_t data)
{
	return;
}

static int is_busy(void)
{
	return 0;
}

ws0010_ret_t oled_init(ws0010_ll_t *ll_funcs)
{
	return WS0010_OK;
}
