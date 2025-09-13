# pdm-experiments
Various PDM experiments

## 001 - simple PDM emitter

PIO based sender fed by a FIFO; receiver is a simple GPIO loop that is 
slow enough to debug by hand.

## 002 - simple PDM emitter with DMA backing

Above - but now fed by a ring buffer made available over DMA.

## 003 - PDM receiver; using 32 bit fifo

Passes on the full bit pattern; no counting yet.

## 004 - PDM receiver; counting the number

Counts the number of bits in the PIO, with Q=32, and then passes this to the FIFO

## 005 - PDM receiver; counting the number

Counts the number of bits in the PIO, with Q=4,5,6,7 or 8=, and then passes this to the FIFO

## 006 - PDM receiver; counting the number

Use an 32 bit fifo to pass 4 counts in one pass
