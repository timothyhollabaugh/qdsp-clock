#include <Arduino.h>
#include <TimeLib.h>
#include <Bounce.h>
#include "Display.h"

#define LED1_PIN 13
#define LED2_PIN 23

#define BUTTON1_PIN 14
#define BUTTON2_PIN 15
#define BUTTON3_PIN 21
#define BUTTON4_PIN 22

#define LIGHT_PIN 16

#define UPDATE_DELAY 100UL

#define BRIGHTNESS_DELAY 10UL
#define BRIGHTNESS_STEP 50

#define BLINK_DELAY 250UL

#define STATE_CLOCK 0
#define STATE_TIME_SET 1

#define HOUR 2
#define MINUTE 1
#define SECOND 0

const int brightness_levels[8] = {
    0,
    33,
    51,
    69,
    102,
    135,
    204,
    255
};

// 4KHz low level
// 490Hz spikes

time_t getTeensy3Time();

void blink_led(unsigned long now_millis);
void board_test(unsigned long now_millis);
void display_test(unsigned long now_millis);

void update_brightness(unsigned long now_millis);
void update(unsigned long now_millis);
int state_clock(unsigned long now_millis, int last_state);
int state_time_set(unsigned long now_millis, int last_state);

void display_time(time_t t);

Bounce alarm_off_button = Bounce(BUTTON1_PIN, 10);
Bounce time_set_button = Bounce(BUTTON2_PIN, 10);
Bounce down_button = Bounce(BUTTON3_PIN, 10);
Bounce up_button = Bounce(BUTTON4_PIN, 10);

Display display = Display();

int brightness_level = 0;

void setup() {

    setSyncProvider(getTeensy3Time);

    Serial.begin(9600);
    //while (!Serial);
    Serial.println("Starting");

    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);

    pinMode(BUTTON1_PIN, INPUT);
    pinMode(BUTTON2_PIN, INPUT);
    pinMode(BUTTON3_PIN, INPUT);
    pinMode(BUTTON4_PIN, INPUT);

    pinMode(LIGHT_PIN, INPUT);

    digitalWrite(BUTTON1_PIN, HIGH);
    digitalWrite(BUTTON2_PIN, HIGH);
    digitalWrite(BUTTON3_PIN, HIGH);
    digitalWrite(BUTTON4_PIN, HIGH);

    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, HIGH);

    display.setup();
}

void loop() {
    unsigned long now_millis = millis();

    //blink_led(now_millis);
    //board_test(now_millis);
    //display_test(now_millis);
    update_brightness(now_millis);
    update(now_millis);
    display.update(now_millis);
}

void update_brightness(unsigned long now) {
    static unsigned long last_update = 0UL;
    static int current_brightness = 0UL;

    if (now - last_update >= BRIGHTNESS_DELAY) {
        int raw_brightness = analogRead(LIGHT_PIN);

        if (raw_brightness > current_brightness + BRIGHTNESS_STEP) {
            current_brightness += BRIGHTNESS_STEP;
        } else if (raw_brightness < current_brightness - BRIGHTNESS_STEP) {
            current_brightness -= BRIGHTNESS_STEP;
        }

        brightness_level = (current_brightness / 150) + 1;
        last_update = now;
    }
}

void update(unsigned long now_millis) {
    static unsigned long last_update = 0UL;

    static int state = STATE_CLOCK;
    static int last_state = state;
    static int state_changed = 0;

    if (now_millis - last_update >= UPDATE_DELAY) {
        int new_state = STATE_CLOCK;

        switch (state) {
            case STATE_CLOCK:
                new_state = state_clock(now_millis, last_state);
                break;
            case STATE_TIME_SET:
                new_state = state_time_set(now_millis, last_state);
                break;
        }

        alarm_off_button.update();
        time_set_button.update();
        down_button.update();
        up_button.update();

        last_state = state;
        state = new_state;
        last_update = now_millis;
    }
}

int state_clock(unsigned long now_millis, int last_state) {

    if (time_set_button.fallingEdge()) {
        return STATE_TIME_SET;
    } else {
        display_time(now());
        return STATE_CLOCK;
    }
}

int state_time_set(unsigned long now_millis, int last_state) {
    static int time_part = HOUR;
    static unsigned long last_blink = 0UL;
    static int blink = 1;
    static TimeElements time_elements;

    if (last_state != STATE_TIME_SET) {
        Serial.println("New State!");
        last_blink = now_millis;
        blink = 1;
        time_part = HOUR;
        breakTime(now(), time_elements);
    }

    display_time(makeTime(time_elements));

    if (blink) {
        switch (time_part) {
            case HOUR:
                display.set_character(0, ' ');
                display.set_character(1, ' ');
                break;
            case MINUTE:
                display.set_character(3, ' ');
                display.set_character(4, ' ');
                break;
            case SECOND:
                display.set_character(6, ' ');
                display.set_character(7, ' ');
                break;
        }
    }

    if (now_millis - last_blink >= BLINK_DELAY) {
        blink = !blink;
        last_blink = now_millis;
    }

    if (up_button.fallingEdge()) {
        last_blink = now_millis;
        blink = 0;
        switch (time_part) {
            case HOUR:
                time_elements.Hour += 1;
                break;
            case MINUTE:
                time_elements.Minute += 1;
                break;
            case SECOND:
                time_elements.Second += 1;
                break;
        }
    }

    if (down_button.fallingEdge()) {
        last_blink = now_millis;
        blink = 0;
        switch (time_part) {
            case HOUR:
                time_elements.Hour -= 1;
                break;
            case MINUTE:
                time_elements.Minute -= 1;
                break;
            case SECOND:
                time_elements.Second -= 1;
                break;
        }
    }

    int next_state = STATE_TIME_SET;

    if (time_set_button.fallingEdge()) {
        switch (time_part) {
            case HOUR:
                time_part = MINUTE;
                blink = 1;
                last_blink = now_millis;
                break;
            case MINUTE:
                time_part = SECOND;
                blink = 1;
                last_blink = now_millis;
                break;
            case SECOND:
                time_part = HOUR;
                blink = 1;
                last_blink = now_millis;

                time_t to_set = makeTime(time_elements);
                setTime(to_set);
                Teensy3Clock.set(to_set);

                /*
                setTime(
                    time_elements.Hour,
                    time_elements.Minute,
                    time_elements.Second,
                    time_elements.Day,
                    time_elements.Month,
                    time_elements.Year
                );
                */

                next_state = STATE_CLOCK;
                break;
        }
    }

    return next_state;

}

void display_time(time_t t) {

    TimeElements current_time;
    breakTime(t, current_time);

    int pm = 0;

    if (current_time.Hour == 0) {
        current_time.Hour = 12;
    } else if (current_time.Hour > 12 ) {
        current_time.Hour -= 12;
        pm = 1;
    }

    char display_string[9];
    sprintf(display_string, "%2d:%02d:%02d", current_time.Hour, current_time.Minute, current_time.Second);

    display.set_text(display_string);

    display.set_brightness(brightness_level);

    if (pm) {
        analogWrite(LED2_PIN, brightness_levels[brightness_level]);
    } else {
        analogWrite(LED2_PIN, 0);
    }
}

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

void blink_led(unsigned long now_millis) {
    static unsigned long last_blink = 0UL;

    if (now_millis - last_blink >= 100UL) {
        int led1_state = digitalRead(LED1_PIN);
        digitalWrite(LED1_PIN, !led1_state);

        //int led2_state = digitalRead(LED2_PIN);
        //digitalWrite(LED2_PIN, !led2_state);

        last_blink = now_millis;
    }
}

void board_test(unsigned long now_millis) {
    static unsigned long last_test = 0UL;

    if (now_millis - last_test >= 1000UL) {
        Serial.print("B1:");
        Serial.print(digitalRead(BUTTON1_PIN), DEC);
        Serial.print("B2:");
        Serial.print(digitalRead(BUTTON2_PIN), DEC);
        Serial.print("B3:");
        Serial.print(digitalRead(BUTTON3_PIN), DEC);
        Serial.print("B4:");
        Serial.print(digitalRead(BUTTON4_PIN), DEC);
        Serial.print("L:");
        Serial.println(analogRead(LIGHT_PIN), DEC);
        last_test = now_millis;
    }
}

void display_test(unsigned long now_millis) {
    static unsigned long last_test = 0UL;
    static char count = 0;

    if (now_millis - last_test >= 1000UL) {
        char text[9];

        sprintf(text, "%03d %d", count, count % 7 + 1);

        display.set_text(text);
        display.set_character(7, count);
        display.set_brightness(count % 7 + 1);

        //display.enable_blink(count % 7 < 4);

        display.set_flash(7, 1);
        display.enable_flash(1);

        count += 1;

        last_test = now_millis;
    }

    display.update(now_millis);
}


