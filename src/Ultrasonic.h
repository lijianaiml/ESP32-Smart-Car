#pragma once

#include <stdint.h>
#include "config.h"

class Ultrasonic {
public:
    Ultrasonic();
    void begin();
    float measureCm();
    float measureAvg();

private:
    float rawMeasure();
};
