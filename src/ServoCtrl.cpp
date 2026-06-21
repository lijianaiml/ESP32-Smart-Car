#include <Arduino.h>
#include "ServoCtrl.h"

static const uint8_t SERVO_CHANNEL = 2;
static const uint32_t SERVO_FREQ = 50;
static const uint8_t SERVO_RES = 16;

static const uint32_t DUTY_MIN = 1638;
static const uint32_t DUTY_MAX = 7864;

ServoCtrl::ServoCtrl() {}

void ServoCtrl::begin() {
    ledcSetup(SERVO_CHANNEL, SERVO_FREQ, SERVO_RES);
    ledcAttachPin(PIN_SERVO, SERVO_CHANNEL);
    center();
}

uint32_t ServoCtrl::angleToDuty(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    return map(angle, 0, 180, DUTY_MIN, DUTY_MAX);
}

void ServoCtrl::setAngle(int angle) {
    ledcWrite(SERVO_CHANNEL, angleToDuty(angle));
}

void ServoCtrl::center() {
    setAngle(SERVO_CENTER);
}

void ServoCtrl::left() {
    setAngle(SERVO_LEFT);
}

void ServoCtrl::right() {
    setAngle(SERVO_RIGHT);
}
