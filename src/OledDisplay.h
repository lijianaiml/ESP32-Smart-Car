#pragma once

#include <stdint.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

class OledDisplay {
public:
    OledDisplay();
    void begin();
    void showBoot();
    void update(const char* state, float dist, const char* bestDir);

private:
    Adafruit_SSD1306* display;
};
