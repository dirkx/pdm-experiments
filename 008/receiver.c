#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pico/stdlib.h"

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "pico/time.h"

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
#define BUFF_BYTES (4 * 1024 * sizeof(uint32_t))
 
uint32_t buffer_rx[BUFF_BYTES/4] __attribute__((aligned(BUFF_BYTES)));

// DMA interrupt handler, called when a DMA channel has transmitted its data
//
uint32_t cnt;
static void dma_irq_handler() {
    dma_irqn_acknowledge_channel(DMA_IRQ_TO_USE, dma_channel_rx);
    const uint32_t n = sizeof(buffer_rx) * 2; 
    cnt += sizeof(buffer_rx) * 2; // 2 counts of 4 bits.

    static uint32_t lst = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (lst) {
	uint32_t d = now - lst;
	printf("PDM Bit counts - rate %.1f kHz\n", 1. * n / d);
    };
    lst = now;

    // 4 bits per Q; Q can be 4 or 8 bits.
    //
    if (0) for(int i = 0; i < sizeof(buffer_rx)/sizeof(*buffer_rx); i++) {
        uint32_t v =  buffer_rx[i];
        v -= 0x88888888; /* we count 0's from 16. */
	printf("\t%4d	sum(%08x) = %d\n", i, v,
		(((((uint8_t *)&v)[0]) >>0) & 0xF) +
		(((((uint8_t *)&v)[0]) >>4) & 0xF) +
		(((((int8_t *)&v)[1]) >>0) & 0xF) +
		(((((uint8_t *)&v)[1]) >>4) & 0xF) +
		(((((uint8_t *)&v)[2]) >>0) & 0xF) +
		(((((uint8_t *)&v)[2]) >>4) & 0xF) +
		(((((uint8_t *)&v)[3]) >>0) & 0xF) +
		(((((uint8_t *)&v)[3]) >>4) & 0xF)
	);
    };
  // Reset DMA process for next block from PIO FIFO
  dma_channel_set_write_addr(dma_channel_rx,buffer_rx,true);
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

#if 0
    channel_config_set_ring(
        &config_rx, 
        true, // Apply to write addresses.
        7 // bits for wrap around; buffer is 256 bytes; 32 transfer
     ); 
#endif

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

    dma_irqn_set_channel_enabled(DMA_IRQ_TO_USE, dma_channel_rx, true);

    pio_sm_set_enabled(pio, sm, true);
#if 0
    // clear FIFO, etc
    pio_sm_clear_fifos(pio, sm);
    pio_sm_restart(pio, sm);
#endif
    dma_channel_start(dma_channel_rx);
}
