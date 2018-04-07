#include <Display.h>

Display::Display() {

}

void Display::setup() {

    pinMode(enable_pin, HIGH);
    digitalWrite(enable_pin, HIGH);

    for (int i = 0; i < 6; i++) {
        pinMode(address_pins[i], OUTPUT);
        digitalWrite(address_pins[i], LOW);
    }

    for (int i = 0; i < 8; i++) {
        pinMode(data_pins[i], OUTPUT);
        digitalWrite(data_pins[i], LOW);
    }
}

void Display::test() {
    write_data(B00110000, 0);

    write_data(B00111000, 'H');
    write_data(B00111001, 'e');
    write_data(B00111010, 'l');
    write_data(B00111011, 'l');
    write_data(B00111100, 'o');
    write_data(B00111101, 'T');
    write_data(B00111110, 'i');
    write_data(B00111111, 'm');
}

void Display::set_text(char text[9]) {
    for (int i = 0; i < 8; i++) {
        if (text[i] != '\0') {
            if (text[i] != character_data[i]) {
                character_data[i] = text[i];
                write_characters = 1;
            }
        } else {
            break;
        }
    }
}

void Display::set_character(char row, char character) {
    if (row < 8 && character_data[row] != character) {
        character_data[row] = character;
        write_characters = 1;
    }
}

void Display::set_flash(char row, char flash) {
    if (row < 8 && flash_data[row] != flash) {
        flash_data[row] = flash;
        write_flash = 1;
    }
}

void Display::set_brightness(char new_brightness) {
    if (new_brightness < 8 && brightness != new_brightness) {
        brightness = new_brightness;
        write_control = 1;
    }
}

void Display::enable_flash(char flash) {
    if (flash_enable != flash) {
        flash_enable = flash;
        write_control = 1;
    }
}

void Display::enable_blink(char blink) {
    if (blink_enable != blink) {
        blink_enable = blink;
        write_control = 1;
    }
}

void Display::update(unsigned long now) {
    if (write_characters) {
        for (int i = 0; i < 8; i++) {
            write_data(character_address | i, character_data[i]);
        }
        write_characters = 0;
    }

    if (write_control) {
        char control_data = 0;
        control_data |= ((brightness ^ B00000111) << 0);
        control_data |= (flash_enable << 3);
        control_data |= (blink_enable << 4);
        write_data(control_address, control_data);
        write_control = 0;
    }

    if (write_flash) {
        char flash_data_byte = 0;
        for (int i = 0; i < 8; i++) {
            flash_data_byte |= flash_data[i] << i;
        }
        write_data(flash_address, flash_data_byte);
        write_flash = 0;
    }

}

void Display::write_data(char address, char data) {
    //digitalWrite(23, HIGH);

    for (int i = 0; i < 6; i++) {
        digitalWrite(address_pins[i], address & (1 << i));
    }

    for (int i = 0; i < 8; i++) {
        digitalWrite(data_pins[i], data & (1 << i));
    }

    digitalWrite(enable_pin, LOW);
    digitalWrite(enable_pin, HIGH);
    //digitalWrite(23, LOW);
}



