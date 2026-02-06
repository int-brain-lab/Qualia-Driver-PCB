#include <Arduino.h>
#include <EEPROM.h>

// Pin definitions
constexpr uint8_t PIN_LED = 1;
constexpr uint8_t PIN_BUTTON_UP = 4;
constexpr uint8_t PIN_BUTTON_DOWN = 3;
constexpr uint8_t PIN_BUTTON_ON = 0;

// Configuration
constexpr uint8_t EEPROM_ADDR_BRIGHTNESS = 0;
constexpr uint8_t BRIGHTNESS_MIN = 2;
constexpr uint8_t BRIGHTNESS_MAX = 255;
constexpr uint8_t BRIGHTNESS_DEFAULT = 128;
constexpr uint16_t FADE_DELAY = 1;
constexpr uint16_t BUTTON_REPEAT_DELAY = 100;
constexpr uint16_t EEPROM_SAVE_DELAY = 2000;

// State
uint8_t brightness = 0;
bool display_on = true;
bool brightness_changed = false;

inline void set_brightness(uint8_t target_brightness) {
  OCR1A = target_brightness;
}

void fade_to_brightness(uint8_t target) {
  uint8_t current = OCR1A;
  int8_t step = (current < target) ? 1 : -1;
  
  while (current != target) {
    current += step;
    set_brightness(current);
    delay(FADE_DELAY);
  }
}

inline bool is_button_pressed(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

void setup() {
  // configure pins
  pinMode(PIN_BUTTON_UP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_DOWN, INPUT_PULLUP);
  pinMode(PIN_BUTTON_ON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  
  // configure PWM
  // with lfuse = 0xE2: 31.25 kHz
  // with lfuse = 0xF1: 62.5 kHz
  TCCR1 = _BV(PWM1A) | _BV(COM1A1) | _BV(CS10);  // 0b11000001 = 0xC1
  OCR1C = 255;
  OCR1A = 0;
  
  // load & set initial brightness
  brightness = EEPROM.read(EEPROM_ADDR_BRIGHTNESS);
  if (brightness < BRIGHTNESS_MIN || brightness > BRIGHTNESS_MAX) {
    brightness = BRIGHTNESS_DEFAULT;
    EEPROM.write(EEPROM_ADDR_BRIGHTNESS, brightness);
  }
  fade_to_brightness(brightness);
}

void loop() {
  static uint32_t last_brightness_change = 0;
  uint32_t current_time = millis();

  if (display_on) {
    // Brightness down
    while (is_button_pressed(PIN_BUTTON_DOWN)) {
      if (brightness > BRIGHTNESS_MIN) {
        brightness--;
        set_brightness(brightness);
        brightness_changed = true;
        last_brightness_change = current_time;
      }
      delay(BUTTON_REPEAT_DELAY);
    }
    
    // Brightness up
    while (is_button_pressed(PIN_BUTTON_UP)) {
      if (brightness < BRIGHTNESS_MAX) {
        brightness++;
        set_brightness(brightness);
        brightness_changed = true;
        last_brightness_change = current_time;
      }
      delay(BUTTON_REPEAT_DELAY);
    }
  }

  // Save brightness to EEPROM
  if (brightness_changed && current_time - last_brightness_change > EEPROM_SAVE_DELAY) {
    EEPROM.write(EEPROM_ADDR_BRIGHTNESS, brightness);
    brightness_changed = false;
  }
  
  // On/Off toggle
  if (is_button_pressed(PIN_BUTTON_ON)) {
    if (display_on) {
      fade_to_brightness(0);
      display_on = false;
    } else {
      fade_to_brightness(brightness);
      display_on = true;
    }
    while (is_button_pressed(PIN_BUTTON_ON)) {
      delay(10);
    }
  }
}
