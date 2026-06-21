#include <Arduino.h>
#include "OledDisplay.h"

static const uint8_t SCREEN_WIDTH = 128;
static const uint8_t SCREEN_HEIGHT = 64;
static const int8_t OLED_RESET = -1;

OledDisplay::OledDisplay() {
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

void OledDisplay::begin() {
    display->begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display->clearDisplay();
    display->display();
}

void OledDisplay::showBoot() {
    display->clearDisplay();
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(10, 20);
    display->println("ESP32 CAR");
    display->setTextSize(1);
    display->setCursor(25, 45);
    display->println("Initializing...");
    display->display();
    delay(1500);
}

void OledDisplay::update(const char* state, float dist, const char* bestDir) {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);

    display->setCursor(0, 0);
    display->print("State: ");
    display->println(state);

    display->setCursor(0, 20);
    display->print("Dist:  ");
    if (dist < 0) {
        display->println("--- cm");
    } else {
        display->print(dist, 1);
        display->println(" cm");
    }

    display->setCursor(0, 40);
    display->print("Best:  ");
    display->println(bestDir);

    display->display();
}
