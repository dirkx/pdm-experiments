#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pico/stdlib.h"

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "receiver.pio.h"

static PIO pio;
static uint offset;
static uint sm;

void init_pdm_receiver(int pin_dat, int pin_clk, uint64_t sampleBitRate) {
    pio = pio1;
    offset = pio_add_program(pio, &receiver_program);
    sm = pio_claim_unused_sm(pio, true);

    receiver_program_init(pio, sm, offset, pin_dat, pin_clk, sampleBitRate);
    pio_sm_set_enabled(pio, sm, true);
}

void loop_receiver(int pin_dat, int pin_clk) {
    for(size_t clk = 0; clk < 8*32*8; clk += 32) {
	uint32_t d32 = receiver_program_get(pio, sm);
	printf("\t\t\t\tQ32: %2d# :: \t\t%08lu\n", clk/32, d32);
        if (clk == 8*32)  printf("== not understood garbange ends ==\n");
    };
}
