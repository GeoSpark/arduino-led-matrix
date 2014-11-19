/* The MIT License (MIT)
 *
 * Copyright (c) 2014 GeoSpark
 *
 * See the LICENSE file for more details, or visit:
 * http://opensource.org/licenses/MIT
 */

#include "led_matrix.h"
#include "conrad_logo.h"
#include <SPI.h>

void initialise_interrupts() {
    cli();
    // Set up TIMER0 to interrupt every 1/125th second.
    TCNT0 = 0;
    // Se the compare register.
    OCR0A = 124;
    // Turn on CTC (timer compare) mode.
    TCCR0A = 1 << WGM01;
    // Set the prescaler to 1024.
    TCCR0B = (1 << CS00) | (1 << CS02);
    // Enable the timer compare interrupt.
    TIMSK0 |= 1 << OCIE0A;
    sei();
}

void swap() {
    // Block until we've reached the end of the current frame.
    while (scanline != 0) {}
    // Disable the TIMER0 interrupt.
    TIMSK0 &= ~(1 << OCIE0A);
    back_buffer = front_buffer;
    front_buffer = buffer + (buffer_size * buffer_num);
    buffer_num %= num_buffers;
    TIMSK0 |= 1 << OCIE0A;
}

void clear_buffer() {
    // We can be pretty sure we're safe here because the rasteriser will be
    // playing about with the front buffer.
    memset((void*)back_buffer, 0, buffer_size);
}

void setup() {
    randomSeed(analogRead(0));

    pinMode(ADDR0_PIN, OUTPUT);
    digitalWrite(ADDR0_PIN, LOW);

    pinMode(ADDR1_PIN, OUTPUT);
    digitalWrite(ADDR1_PIN, LOW);

    pinMode(ADDR2_PIN, OUTPUT);
    digitalWrite(ADDR2_PIN, LOW);

    pinMode(MODE_PIN, OUTPUT);
    digitalWrite(MODE_PIN, LOW);

    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider((F_CPU + 500000L) / 1000000L);

    // We can safely do this here because we've not enabled the TIMER0 interrupt.
    memset((void*)buffer, 0, buffer_size * num_buffers);

    initialise_interrupts();
}

// Routine to strobe the scanlines.
ISR(TIMER0_COMPA_vect) {
    digitalWrite(MODE_PIN, HIGH);
    PORT = (PORT & ADDR_SELECT_MASK) | (scanline << ADDR_SELECT_SHIFT);
    digitalWrite(MODE_PIN, LOW);

    volatile uint8_t* buffer_pos = (uint8_t*)(front_buffer + (scanline * matrix_width * scanline_length));

    for (uint8_t i = 0; i < scanline_length; ++i) {
        SPI.transfer(buffer_pos[i]);
    }

    scanline = (scanline + 1) % num_scanlines;
}

void set_pixel(const uint8_t x, const uint8_t y, const pixel_t pixel) {
    volatile pixel_t* address = back_buffer + x + (y * matrix_width);
    address->r = pixel.r;
    address->g = pixel.g;
    address->b = pixel.b;
}

void scroll_left() {
    memcpy((pixel_t*)back_buffer, (pixel_t*)front_buffer + 1, (matrix_width * matrix_height * sizeof(pixel_t)) - 1);

    for (uint8_t y = 1; y <= matrix_height; ++y) {
        uint16_t index = (y * matrix_width) - 1;
        back_buffer[index].r = 0;
        back_buffer[index].g = 0;
        back_buffer[index].b = 0;
    }
}

void draw_bitmap(const char* image) {
    for (uint16_t i = 0; i < matrix_width * matrix_height; ++i) {
        HEADER_PIXEL(image, back_buffer[i]);
    }
}

void loop() {
    static bool show_logo = true;

    clear_buffer();

    if (show_logo) {
        show_logo = false;
        draw_bitmap(conrad_logo);
        swap();
        delay(1000);
    }

    pixel_t colour;
    colour.r = random(256);
    colour.g = random(256);
    colour.b = random(256);
    scroll_left();
    set_pixel(14, random(15), colour);

    swap();
}
