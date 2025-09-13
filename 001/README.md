# 001 - simple PIO

PIO loop that reads data from a FIFO; and shifts that out in the rithm of the clock.

## Wiring

Generator (i.e. simulates a microphone):

* GPIO 2 -- DAT output
* GPIO 3 -- Clock input

Reader (i.e. simulates what a PDM to USB interface would do):

* GPIO 4 -- DAT input
* GPIO 5 -- Clock output

So 2-4 and 4-5 need to be wired together for this to work.


