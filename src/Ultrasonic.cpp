#include <Arduino.h>
#include "Ultrasonic.h"

Ultrasonic::Ultrasonic() {}

void Ultrasonic::begin() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIG, LOW);
}

float Ultrasonic::rawMeasure() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    uint32_t duration = pulseIn(PIN_ECHO, HIGH, MEASURE_TIMEOUT);

    if (duration == 0) {
        return -1.0f;
    }

    float distance = duration * 0.0343f / 2.0f;

    if (distance > 400.0f || distance <= 0.0f) {
        return -1.0f;
    }

    return distance;
}

float Ultrasonic::measureCm() {
    return rawMeasure();
}

float Ultrasonic::measureAvg() {
    float sum = 0.0f;
    uint8_t validCount = 0;
    float maxVal = -1.0f;

    for (uint8_t i = 0; i < MEASURE_AVG_N; i++) {
        float val = rawMeasure();
        if (val >= 0.0f) {
            sum += val;
            if (val > maxVal) {
                maxVal = val;
            }
            validCount++;
        }
        delay(10);
    }

    if (validCount == 0) {
        return -1.0f;
    }

    if (validCount >= 2) {
        sum -= maxVal;
        validCount--;
    }

    return sum / static_cast<float>(validCount);
}
