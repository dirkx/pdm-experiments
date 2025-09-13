# 002 - simple PIO via DMA

One PIO loop that reads a DMA-ed ringbuffer; and shifts that out in the rithm of the clock.

Second PIO loop that sends a clock to the first; and reads out a bit at the time.

These bits are then counted; and every 8 clock-cycles(for Q=8) the count is shifted into a 32 bit FIFO. 

Once we have 32 of these - the result is DMA'd out.

Problem - not restarting

## Wiring

Generator (i.e. simulates a microphone):

* GPIO 2 -- DAT output
* GPIO 3 -- Clock input

Reader (i.e. simulates what a PDM to USB interface would do):

* GPIO 4 -- DAT input
* GPIO 5 -- Clock output

So 2-4 and 4-5 need to be wired together for this to work.


