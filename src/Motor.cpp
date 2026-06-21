#include <Arduino.h>
#include "Motor.h"
#include "config.h"

static const uint8_t CHANNEL_A = 0;
static const uint8_t CHANNEL_B = 1;

Motor::Motor() {}

void Motor::begin() {
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_BIN1, OUTPUT);
    pinMode(PIN_BIN2, OUTPUT);

    ledcSetup(CHANNEL_A, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
    ledcSetup(CHANNEL_B, MOTOR_PWM_FREQ, MOTOR_PWM_RES);

    ledcAttachPin(PIN_PWMA, CHANNEL_A);
    ledcAttachPin(PIN_PWMB, CHANNEL_B);

    stop();
}

void Motor::setA(int dir, uint8_t speed) {
    if (dir == 0) {
        digitalWrite(PIN_AIN1, LOW);
        digitalWrite(PIN_AIN2, LOW);
        ledcWrite(CHANNEL_A, 0);
    } else if (dir == 1) {
        digitalWrite(PIN_AIN1, LOW);
        digitalWrite(PIN_AIN2, HIGH);
        ledcWrite(CHANNEL_A, speed);
    } else {
        digitalWrite(PIN_AIN1, HIGH);
        digitalWrite(PIN_AIN2, LOW);
        ledcWrite(CHANNEL_A, speed);
    }
}

void Motor::setB(int dir, uint8_t speed) {
    if (dir == 0) {
        digitalWrite(PIN_BIN1, LOW);
        digitalWrite(PIN_BIN2, LOW);
        ledcWrite(CHANNEL_B, 0);
    } else if (dir == 1) {
        digitalWrite(PIN_BIN1, HIGH);
        digitalWrite(PIN_BIN2, LOW);
        ledcWrite(CHANNEL_B, speed);
    } else {
        digitalWrite(PIN_BIN1, LOW);
        digitalWrite(PIN_BIN2, HIGH);
        ledcWrite(CHANNEL_B, speed);
    }
}

void Motor::forward(uint8_t speed) {
    setA(1, speed);
    setB(1, speed);
}

void Motor::backward(uint8_t speed) {
    setA(-1, speed);
    setB(-1, speed);
}

void Motor::turnLeft(uint8_t speed) {
    setA(-1, speed);
    setB(1, speed);
}

void Motor::turnRight(uint8_t speed) {
    setA(1, speed);
    setB(-1, speed);
}

void Motor::stop() {
    setA(0, 0);
    setB(0, 0);
}
