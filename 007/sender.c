#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pico/stdlib.h"

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "pdmgenerate.pio.h"

#ifndef DMA_IRQ_PRIORITY
#define DMA_IRQ_PRIORITY PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY
#endif

#ifndef PIO_IRQ_PRIORITY
#define PIO_IRQ_PRIORITY PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY
#endif

#define PIO_IRQ_TO_USE 0
#define DMA_IRQ_TO_USE 0

static uint dma_channel;
uint32_t buffer_tx[32] __attribute__((aligned(256)));

static void dma_irq_handler() {
    dma_irqn_acknowledge_channel(DMA_IRQ_TO_USE, dma_channel);

    // Reset the buffer to the start.
    //
    dma_channel_hw_addr(dma_channel)->al3_read_addr_trig = (uintptr_t) buffer_tx;
}

void init_pdm_sender(int pin_dat, int pin_clk) {
#if 0
    // counter per 8 bits
    for(size_t i = 0; i < sizeof(buffer_tx); i++) 
        ((uint8_t*)buffer_tx)[i]=i;
#endif
    // increasing # of bits/32 word.
    for(size_t i = 0; i < sizeof(buffer_tx)/4; i++) {
        buffer_tx[i]= 	0x55555555; 
        buffer_tx[i]= 	0x11111111; 
        buffer_tx[i]= 	0x01010101; 
        buffer_tx[i]= 	0x10101010; 
        // buffer_tx[i]= (1<<(i % 32));
        //buffer_tx[i]= 0xFF00FF00; 
        // buffer_tx[i]= 0x0FFFFFF0;  
        //buffer_tx[i]= -1  & (-1 << ((2*i) % 32));
        buffer_tx[i]= 0xFFFEFFFF;  
        printf("%4d: %08x %d\n", i, buffer_tx[i],__builtin_popcount(buffer_tx[i]));
    };


    PIO pio = pio0;
    uint offset = pio_add_program(pio, &pdmgenerate_program);
    uint sm = pio_claim_unused_sm(pio, true);

    pdmgenerate_program_init(pio, sm, offset, pin_dat, pin_clk);
    pio_sm_set_enabled(pio, sm, true);

    irq_add_shared_handler(dma_get_irq_num(DMA_IRQ_TO_USE), dma_irq_handler, DMA_IRQ_PRIORITY);
    irq_set_enabled(dma_get_irq_num(DMA_IRQ_TO_USE), true);

    dma_channel = dma_claim_unused_channel(true);
printf("TX dma: %d\n", dma_channel);

    dma_channel_config config_tx = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&config_tx, DMA_SIZE_32);
    channel_config_set_read_increment(&config_tx, true);
    channel_config_set_write_increment(&config_tx, false);

    // Loop; 4 -> 16/4 =  4x32 (uint32_t) repeat
    //       5 -> 32/4 =  8x32 repeat
    //       6 -> 64/4 = 16x32 repeat cycle, etc
    //       7 ->128/4 = 32x32 repeat cycle, etc
    channel_config_set_ring(
	&config_tx, 
	false, 
	7 // bits for wrap around; buffer is 256 bytes; 32 transfer
   ); 

    // setup dma to write to pio fifo
    //
    channel_config_set_dreq(&config_tx, pio_get_dreq(pio, sm, true));
    dma_channel_configure(
	dma_channel, 
	&config_tx, 
	&pio->txf[sm],  	// initial write address
	buffer_tx,     		// first read address
#if 0
        sizeof(buffer_tx)/4,    // number of 32 bit transfers
#else
	-1, 			// largest number we can fill out here, rely on the DMA irq to reset.
#endif
	true // start immediately
    );

    // enable irq for tx - so we can reset after the transaction count (-1) is hit.
    dma_irqn_set_channel_enabled(DMA_IRQ_TO_USE, dma_channel, true);
}
