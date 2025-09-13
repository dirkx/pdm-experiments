#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pico/stdlib.h"

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"

#include "pdmgenerate.pio.h"

#define BUF_SIZE 8

#define PIO_GPIO_DAT 2
#define PIO_GPIO_CLK 3

#define GPIO_DAT 4
#define GPIO_CLK 5

int main() {
    stdio_init_all();
    printf("Starting %s " __DATE__ " " __TIME__ "\n", rindex(__FILE__,'/')+1);

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &pdmgenerate_program);
    uint sm = pio_claim_unused_sm(pio, true);

    pdmgenerate_program_init(pio, sm, offset, PIO_GPIO_DAT, PIO_GPIO_CLK);
    pio_sm_set_enabled(pio, sm, true);

    pdmgenerate_program_putc(pio, sm, 0x04030201);
    pdmgenerate_program_putc(pio, sm, 0x08070605);

#if 0
    gpio_init(PIO_GPIO_DAT);
    gpio_set_dir(PIO_GPIO_DAT, GPIO_OUT);
    gpio_put(PIO_GPIO_DAT,1);
#endif

    gpio_init(GPIO_DAT);
    gpio_set_dir(GPIO_DAT, GPIO_IN);

    gpio_init(GPIO_CLK);
    gpio_set_dir(GPIO_CLK, GPIO_OUT);

    uint8_t d = 0;
    for(int clk = 0; clk < 10*8; clk++) {
       gpio_put(GPIO_CLK,1);
//       sleep_ms(1);
       gpio_put(GPIO_CLK,0);
 //      sleep_ms(1);
       // printf("CLK: %2d - %2d\n", clk, gpio_get(GPIO_DAT));
       printf("%2d/%2d ", clk, gpio_get(GPIO_DAT));
       d = (d>>1) | (gpio_get(GPIO_DAT)<<7);
       if (clk % 8 == 7)
          printf("    Q8: %2d# :: 0x%x\n", clk/8, d);
//       sleep_ms(1);
    };

    return 0;
}

