#ifndef DISPLAY_H
#define DISPLAY_H

#include "Arduino.h"

class Display {
    public:
        Display();

        // Setup all the output pins
        void setup();

        // do some testing
        void test();

        // Set all of the text on the display at once
        // text: string of 8 characters to display
        void set_text(char* text);

        // Set a single character on the display
        // row: the row to set
        // character: the character to set
        void set_character(char row, char character);

        // Set a character to flash
        // row: the row to flash
        // flash: 1 -> flash, 0 -> no flash
        void set_flash(char row, char flash);

        // Set the brightness of the display
        // brightness: 0 -> off, 8 -> full brightness
        void set_brightness(char brightness);

        // Enable or disable blinking of the entire display
        // blink: 0 -> no blink, 1 -> blink
        void enable_blink(char blink);

        // Enable or disable flashing of individual characters
        // Set charasters to flash with set_flash()
        // flash: 0 -> no flash, 1 -> flash
        void enable_flash(char flash);

        // Update the display
        // now: the current time in millis
        void update(unsigned long now);

    private:
        const int enable_pin = 8;
        const int address_pins[6] = {3, 4, 5, 6, 7, 2};
        const int data_pins[8] = {12, 11, 10, 9, 17, 18, 19, 20};

        const char character_address = B00111000;
        char character_data[9] = "        ";
        int write_characters = 0;

        const char control_address = B00110000;
        char brightness;
        char flash_enable;
        char blink_enable;
        int write_control = 0;

        const char flash_address = B00000000;
        char flash_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        int write_flash = 0;

        void write_data(char address, char data);
};

#endif // DISPLAY_H
