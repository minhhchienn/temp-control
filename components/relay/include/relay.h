#ifndef __RELAY__
#define __RELAY__
#include <stdint.h>

static const uint8_t relay1_num = 2;
static const uint8_t relay2_num = 5;

static int8_t relay1_state = 0;
static int8_t relay2_state = 0;

void init_relay();
void toggle_relay(uint8_t relay_number);

#endif