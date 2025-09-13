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

#ifndef DMA_IRQ_PRIORITY
#define DMA_IRQ_PRIORITY PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY
#endif

#ifndef PIO_IRQ_PRIORITY
#define PIO_IRQ_PRIORITY PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY
#endif

#define PIO_IRQ_TO_USE 1
#define DMA_IRQ_TO_USE 1

static uint dma_channel_rx;
static unsigned char buffer_rx[32];

// DMA interrupt handler, called when a DMA channel has transmitted its data
static void dma_irq_handler() {
    dma_irqn_acknowledge_channel(DMA_IRQ_TO_USE, dma_channel_rx);

    // Reset the buffer to the start.
    //
    dma_channel_hw_addr(dma_channel_rx)->al3_read_addr_trig = (uintptr_t) buffer_rx;
    printf("dma_rx complete\n");
}

void init_pdm_receiver(int pin_dat, int pin_clk, uint64_t sampleBitRate) {
    pio = pio1;
    offset = pio_add_program(pio, &receiver_program);
    sm = pio_claim_unused_sm(pio, true);

#if 0 
    irq_add_shared_handler(dma_get_irq_num(DMA_IRQ_TO_USE), dma_irq_handler, DMA_IRQ_PRIORITY);
    irq_set_enabled(dma_get_irq_num(DMA_IRQ_TO_USE), true);

    dma_channel_rx = dma_claim_unused_channel(false);
    if (dma_channel_rx < 0) {
        panic("No free dma channels");
    }
    dma_channel_config config_rx = dma_channel_get_default_config(dma_channel_rx);

    channel_config_set_transfer_data_size(&config_rx, DMA_SIZE_8);
    channel_config_set_read_increment(&config_rx, false);
    channel_config_set_write_increment(&config_rx, true);

    dma_irqn_set_channel_enabled(DMA_IRQ_TO_USE, dma_channel_rx, true);

    // setup dma to read from pio fifo
    channel_config_set_dreq(&config_rx, pio_get_dreq(pio, sm, false));
    // 8-bit read from the uppermost byte of the FIFO, as data is left-justified 
    // so need to add 3. Don't forget the cast!
    dma_channel_configure(
	dma_channel_rx, 
	&config_rx, 
	buffer_rx, 
	(io_rw_8*)&pio->rxf[sm],
	sizeof(buffer_rx),
	true // dma started
    );
#endif

    receiver_program_init(pio, sm, offset, pin_dat, pin_clk, sampleBitRate);
    pio_sm_set_enabled(pio, sm, true);
}

void loop_receiver(int pin_dat, int pin_clk) {
    for(size_t clk = 0; clk < 8*32*8; clk += 32) {
	uint32_t d32 = receiver_program_get(pio, sm);
	printf("\t\t\t\tQ32: %2d# :: \t\t%d\n", clk/32, d32);
        if (clk == 8*32)  printf("== not understood garbange ends ==\n");
    };
}
