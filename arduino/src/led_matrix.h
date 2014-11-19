/* The MIT License (MIT)
 *
 * Copyright (c) 2014 GeoSpark
 *
 * See the LICENSE file for more details, or visit:
 * http://opensource.org/licenses/MIT
 */

#ifndef __LED_MATRIX_H__
#define __LED_MATRIX_H__

#include <WString.h>

#define MODE_PIN 7
#define ADDR0_PIN 8
#define ADDR1_PIN 9
#define ADDR2_PIN 10

#define PORT PORTB
#define ADDR_SELECT_MASK 0b11111000
#define ADDR_SELECT_SHIFT 0

// Assume for now that we're using hardware SPI cos it's easier.
#define CLK_PIN 13
#define DATA_PIN 11

/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */
#define HEADER_PIXEL(data,pixel) {\
pixel.r = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel.g = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel.b = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}

struct pixel_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} __attribute__ ((packed));

const uint8_t num_scanlines = 5;
const uint8_t matrix_width = 15;
const uint8_t scanline_length = matrix_width / num_scanlines;
const uint8_t matrix_height = 15;
const uint8_t num_buffers = 2;
const uint16_t buffer_size = matrix_width * matrix_height;

volatile pixel_t buffer[buffer_size * num_buffers];
volatile uint8_t buffer_num = 0;
volatile uint8_t scanline = 0;
volatile pixel_t* front_buffer = buffer;
volatile pixel_t* back_buffer = buffer + buffer_size;

void initialise_interrupts();
void swap();
void clear_buffer();
void set_pixel(const uint8_t x, const uint8_t y, const pixel_t pixel);
void draw_bitmap(const char* image);

#endif
