#pragma once

#include <stdint.h>
#include "config.h"

class Motor {
public:
    Motor();
    void begin();
    void forward(uint8_t speed = DEFAULT_SPEED);
    void backward(uint8_t speed = DEFAULT_SPEED);
    void turnLeft(uint8_t speed = TURN_SPEED);
    void turnRight(uint8_t speed = TURN_SPEED);
    void stop();

private:
    void setA(int dir, uint8_t speed);
    void setB(int dir, uint8_t speed);
};
