/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "ringbuffer.h"
#include <stdio.h>

/// \tag::uart_advanced[]

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

static int chars_rxed = 0;
// -----------------------------------------------------------------------
// Serial IO Ringbuffers.
// TX and RX are relative to the Pico 
// -----------------------------------------------------------------------
#define RB_TX_SIZE 256 
#define RB_RX_SIZE 256 

RINGBUF rb_tx;
uint8_t rb_tx_buf[RB_TX_SIZE];

RINGBUF rb_rx;
uint8_t rb_rx_buf[RB_RX_SIZE];

// -----------------------------------------------------------------------

int __sapi_puts(uint8_t *s) {
  while(*s) SAPI_StreamPutChar(*s++);
  }

// -----------------------------------------------------------------------
// UART interrupt handler
// -----------------------------------------------------------------------
int count_uart_txed = 0;
int count_uart_rxed = 0;

void uart_handler() {
  
    // TX Handler.
    if ( ringbuffer_used(&rb_tx) ) {
      if ( uart_is_writable(UART_ID) ) {
        uart_putc(UART_ID,ringbuffer_getchar(&rb_tx));
        count_uart_txed++;
      }
      if ( ringbuffer_used(&rb_tx) == 0 ) {
        // Turn off the TX IRQ 
        // Now enable the UART to send interrupts - RX only
        uart_set_irq_enables(UART_ID, true, false);
      }
      else { 
        uart_set_irq_enables(UART_ID, true, true);
      }
  
    }

    // RX Handler
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        ringbuffer_addchar(&rb_rx, ch);
        count_uart_rxed++;
    }
    // 
    // Tell somebody that there is data here.
    // 
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
int main() {

    char buf[40];
  
    // Initialize the UART ringbuffers. 
    ringbuffer_init(&rb_tx,rb_tx_buf,sizeof(rb_tx_buf));
    ringbuffer_init(&rb_rx,rb_rx_buf,sizeof(rb_rx_buf));

    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 2400);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    uart_puts(UART_ID, "\nHello, uart interrupts\n");
    sprintf(buf, "rbused=%d\n", ringbuffer_used(&rb_rx));
    uart_puts(UART_ID, buf);
  
    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, uart_handler);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    int count = 0;
    while (1) {
      char buf[40];
      int uart_rxd = ringbuffer_used(&rb_rx);
      sprintf(buf, "Hello #%d, %d %d \n", count++, count_uart_txed, count_uart_rxed);
      __sapi_puts(buf);
      gpio_put(LED_PIN, count & 1);
      sleep_ms(500);
      }
}

/// \end:uart_advanced[]
