#pragma once

#include <stdint.h>
#include "hardware/pio.h"

uint32_t nec_encode_frame(uint8_t address, uint8_t data);
int nec_tx_init(PIO pio, uint pin);
bool send_nec(uint8_t address, uint8_t command); 