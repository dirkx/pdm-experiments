# pdm-experiments
Various PDM experiments

## 001 - simple PDM emitter

PIO based sender fed by a FIFO; receiver is a simple GPIO loop that is 
slow enough to debug by hand.

## 002 - simple PDM emitter with DMA backing

Above - but now fed by a ring buffer made available over DMA.
