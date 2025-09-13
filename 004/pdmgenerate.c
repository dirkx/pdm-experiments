#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pico/stdlib.h"

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "pdmgenerate.pio.h"

#include "sender.h"
#include "receiver.h"

#define PIO_GPIO_DAT 2
#define PIO_GPIO_CLK 3

#define GPIO_DAT 4
#define GPIO_CLK 5

#define Mhz 1000 * 1000

int main() {
    stdio_init_all();
    printf("Starting %s " __DATE__ " " __TIME__ "\n", rindex(__FILE__,'/')+1);

    init_pdm_sender(PIO_GPIO_DAT, PIO_GPIO_CLK);
    init_pdm_receiver(GPIO_DAT, GPIO_CLK, 4 * Mhz);

    // dma_channel_wait_for_finish_blocking(dma_channel);

    loop_receiver(GPIO_DAT, GPIO_CLK);

    return 0;
}

