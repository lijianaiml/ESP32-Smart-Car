#pragma once

#include <stdint.h>
#include "config.h"

class ServoCtrl {
public:
    ServoCtrl();
    void begin();
    void setAngle(int angle);
    void center();
    void left();
    void right();

private:
    uint32_t angleToDuty(int angle);
};
