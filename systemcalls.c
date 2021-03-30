//
// System Calls 
//
//

#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "ringbuffer.h"

extern RINGBUF rb_tx;
extern RINGBUF rb_rx;

#define UART_ID uart0

// System Call Stand-in.

// ---------------------------------------------
// ---------------------------------------------
int __SAPI_02_PutChar(uint8_t c) {
  ringbuffer_addchar(&rb_tx, c);

  // Enable the uart tx IRQ 
  uart_set_irq_enables(UART_ID, true, true);
  
  return(ringbuffer_free(&rb_tx));
  }

// ---------------------------------------------
// ---------------------------------------------
// Get the next char from the Queue.
int __SAPI_03_GetChar(uint8_t c) {
  return(ringbuffer_getchar(&rb_rx));
  }

// ---------------------------------------------
// ---------------------------------------------
// Return the number of bytes available in the Queue
int __SAPI_04_GetCharAvail() {
  return(ringbuffer_used(&rb_rx));
  }




