#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pico/stdlib.h"

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "receiver.pio.h"

#if 0
void init_pdm_receiver(int pin_dat, int pin_clk) {
    gpio_init(pin_dat);
    gpio_set_dir(pin_dat, GPIO_IN);

    gpio_init(pin_clk);
    gpio_set_dir(pin_clk, GPIO_OUT);
}

void loop_receiver(int pin_dat, int pin_clk) {
    uint8_t d8 = 0;
    uint16_t d16 = 0;
    uint32_t d32 = 0;
    for(int clk = 0; 1; clk++) {
       gpio_put(pin_clk,0);
       gpio_put(pin_clk,1);

       // printf("CLK: %2d - %2d\n", clk, gpio_get(pin_dat));
if(0)
	printf("%2d ", gpio_get(pin_dat));

       d8  = ( d8>>1) | (gpio_get(pin_dat)<<7);
       d16 = (d16>>1) | (gpio_get(pin_dat)<<15);
       d32 = (d32>>1) | (gpio_get(pin_dat)<<31);
if(0)
       if (clk % 8 == 7)
          printf("\tQ08: %2d# :: 0x%02x\n", clk/8, d8);
if(0)
       if (clk % 16 == 15)
          printf("\t\t\t\tQ16: %2d# :: \t0x%04x\n", clk/16, d16);
       if (clk % 32 == 31)
          printf("\t\t\t\tQ32: %2d# :: \t\t0x%08x\n", clk/32, d32);
    };
}
#endif

#if  1
PIO pio;
uint offset;
uint sm;

void init_pdm_receiver(int pin_dat, int pin_clk, uint64_t sampleBitRate) {
    pio = pio1;
    offset = pio_add_program(pio, &receiver_program);
    sm = pio_claim_unused_sm(pio, true);

    receiver_program_init(pio, sm, offset, pin_dat, pin_clk, sampleBitRate);
    pio_sm_set_enabled(pio, sm, true);
    printf("Start clockig\n");
}

void loop_receiver(int pin_dat, int pin_clk) {
    uint8_t d8 = 0;
    uint16_t d16 = 0;
    uint32_t d32 = 0;
    printf("Waiting for FIFO input\n");
    for(size_t clk = 0; clk < 8*32*8; clk += 32) {
	uint32_t d32 = receiver_program_get(pio, sm);
        printf("\t\t\t\tQ32: %2d# :: \t\t%08lu\n", clk/32, d32);
    };
}
#endif

