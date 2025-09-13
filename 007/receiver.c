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

#ifndef DMA_IRQ_TO_USE
// (DMA_IRQ_1)
#define DMA_IRQ_TO_USE 1 
#endif

static uint dma_channel_rx;
uint32_t buffer_rx[32] __attribute__((aligned(256)));

// DMA interrupt handler, called when a DMA channel has transmitted its data
//
static void dma_irq_handler() {
    dma_irqn_acknowledge_channel(DMA_IRQ_TO_USE, dma_channel_rx);

    printf("dma_rx complete\n");
    for(int i = 0; i < sizeof(buffer_rx)/sizeof(*buffer_rx); i++) {
        uint32_t v =  buffer_rx[i];
        v -= 0x08080808;
	printf("DMA: %4d	count = %08x\n", i, v);
    };

  // Reset DMA process for next block from PIO FIFO
  // dma_channel_set_write_addr(dma_channel_rx,buffer_rx,true);
  dma_channel_hw_addr(dma_channel_rx)->al2_write_addr_trig = (uintptr_t) buffer_rx;
}

void init_pdm_receiver(int pin_dat, int pin_clk, uint64_t sampleBitRate) {
    pio = pio1;
    offset = pio_add_program(pio, &receiver_program);
    sm = pio_claim_unused_sm(pio, true);
    receiver_program_init(pio, sm, offset, pin_dat, pin_clk, sampleBitRate);

    irq_add_shared_handler(dma_get_irq_num(DMA_IRQ_TO_USE), dma_irq_handler, DMA_IRQ_PRIORITY);
    irq_set_enabled(dma_get_irq_num(DMA_IRQ_TO_USE), true);

    dma_channel_config config_rx;
    dma_channel_rx = dma_claim_unused_channel(true);
    config_rx = dma_channel_get_default_config(dma_channel_rx);

    channel_config_set_transfer_data_size(&config_rx, DMA_SIZE_32);

    channel_config_set_read_increment(&config_rx, false); // do not increment; FIFO
    channel_config_set_write_increment(&config_rx, true); 

    // Wire DMA to FIFO
    channel_config_set_dreq(&config_rx, pio_get_dreq(pio, sm, false /* is rx */));

    // 8-bit read from the uppermost byte of the FIFO, as data is left-justified 
    // so need to add 3. Don't forget the cast!
    dma_channel_configure(
	dma_channel_rx, 
	&config_rx, 
	buffer_rx,  // write addr
	(io_rw_8*)&pio->rxf[sm], // read addr
	sizeof(buffer_rx) / sizeof(*buffer_rx), // -- number of transactions
	true // dma started
    );
#if 0
    channel_config_set_ring(
        &config_rx, 
        true, // Apply to write addresses.
        7 // bits for wrap around; buffer is 256 bytes; 32 transfer
     ); 
#endif

    dma_irqn_set_channel_enabled(DMA_IRQ_TO_USE, dma_channel_rx, true);

    pio_sm_set_enabled(pio, sm, true);
#if 0
    // clear FIFO, etc
    pio_sm_clear_fifos(pio, sm);
    pio_sm_restart(pio, sm);
#endif
}

void loop_receiver(int pin_dat, int pin_clk) {
//    dma_channel_start(dma_channel_rx);
    dma_channel_wait_for_finish_blocking(dma_channel_rx);

    return;
}
