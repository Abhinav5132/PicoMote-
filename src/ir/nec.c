#include "nec.h"
#include "hardware/clocks.h"
#include "nec_burst_control.pio.h"
#include "nec_carrier_control.pio.h"


static PIO tx_pio;
static int tx_sm = -1; 

uint32_t nec_encode_frame(uint8_t address, uint8_t data) {
    return (uint32_t)address
        | ((uint32_t)(uint8_t)~address << 8)
        | ((uint32_t)data << 16)
        | ((uint32_t)(uint8_t)~data << 24);
}

int nec_tx_init(PIO pio, uint pin) {
    uint carrier_burst_offset; 
    if (pio_can_add_program(pio, &nec_burst_control_program)) {
        carrier_burst_offset = pio_add_program(pio, &nec_burst_control_program); 
    } else {
        return -1; 
    }

    int carrier_burst_sm = pio_claim_unused_sm(pio, true); 
    if (carrier_burst_sm == -1) {
        return -1; 
    }

    nec_burst_control_init(pio, carrier_burst_sm, carrier_burst_offset, pin, 38.222e3); 
    
    uint carrier_control_offset; 
    if (pio_can_add_program(pio, &nec_carrier_control_program)){
        carrier_control_offset = pio_add_program(pio, &nec_carrier_control_program); 
    } else {
        return -1; 
    }


    int carrier_control_sm = pio_claim_unused_sm(pio, true); 
    if (carrier_control_sm == -1) {
        return -1; 
    }

    nec_carrier_control_init(pio, carrier_control_sm, carrier_control_offset, 2 * (1 / 562.5e-6f), 32); 

    tx_pio = pio;
    tx_sm = carrier_control_sm; 
    return carrier_control_sm; 
}

bool send_nec(uint8_t address, uint8_t command) {
    if (tx_sm < 0) return false;

    uint sm = (uint)tx_sm; 
    if(pio_sm_is_tx_fifo_full(tx_pio, sm)) return false; 
    
    pio_sm_put(tx_pio, sm, nec_encode_frame(address, command)); 

    return true; 
}
