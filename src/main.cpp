#include <Arduino.h>
#include "config.h"
#include "Motor.h"
#include "ServoCtrl.h"
#include "Ultrasonic.h"
#include "OledDisplay.h"
#include "WiFiControl.h"

enum class State {
    AUTO_FORWARD,
    AUTO_SCANNING,
    AUTO_TURNING,
    AUTO_BACKWARD,
    MANUAL,
    STOPPED
};

static Motor motor;
static ServoCtrl servo;
static Ultrasonic ultrasonic;
static OledDisplay display;
static WiFiControl wifi;

static State state = State::AUTO_FORWARD;
static bool manualMode = false;
static const char* bestDir = "FWD";
static float currentDist = -1.0f;

static const char* stateToString(State s) {
    switch (s) {
        case State::AUTO_FORWARD:   return "AUTO_FWD";
        case State::AUTO_SCANNING:  return "AUTO_SCAN";
        case State::AUTO_TURNING:   return "AUTO_TURN";
        case State::AUTO_BACKWARD:  return "AUTO_BACK";
        case State::MANUAL:         return "MANUAL";
        case State::STOPPED:        return "STOPPED";
        default:                    return "UNKNOWN";
    }
}

void setup() {
    Serial.begin(115200);
    motor.begin();
    servo.begin();
    servo.center();
    ultrasonic.begin();
    display.begin();
    display.showBoot();

    wifi.begin("ESP32_CAR", "12345678");
    Serial.println("[WiFi] Start AP: ESP32_CAR, Password: 12345678");
    Serial.println("[WiFi] Default: Auto Avoidance Mode");

    state = State::AUTO_FORWARD;
    manualMode = false;
    bestDir = "FWD";
    delay(500);
}

void loop() {
    static uint32_t lastLoop = 0;
    if (millis() - lastLoop < LOOP_INTERVAL_MS) return;
    lastLoop = millis();

    wifi.handle();

    WiFiControl::Command cmd = wifi.getCommand();

    if (cmd != WiFiControl::Command::NONE) {
        Serial.printf("[WiFi] CMD: %d\n", (int)cmd);
        switch (cmd) {
            case WiFiControl::Command::FORWARD:
                manualMode = true;
                motor.forward();
                state = State::MANUAL;
                break;
            case WiFiControl::Command::BACKWARD:
                manualMode = true;
                motor.backward();
                state = State::MANUAL;
                break;
            case WiFiControl::Command::LEFT:
                manualMode = true;
                motor.turnLeft();
                state = State::MANUAL;
                break;
            case WiFiControl::Command::RIGHT:
                manualMode = true;
                motor.turnRight();
                state = State::MANUAL;
                break;
            case WiFiControl::Command::STOP:
                manualMode = false;
                motor.stop();
                state = State::STOPPED;
                Serial.println("[WiFi] Car Stopped");
                break;
            case WiFiControl::Command::AUTO:
                manualMode = false;
                motor.stop();
                state = State::AUTO_FORWARD;
                Serial.println("[WiFi] Switch to Auto Mode");
                break;
            default:
                break;
        }
        wifi.clearCommand();
        display.update(stateToString(state), currentDist, bestDir);
        return;
    }

    if (state == State::STOPPED) {
        motor.stop();
        display.update(stateToString(state), currentDist, bestDir);
        return;
    }

    if (manualMode) {
        display.update(stateToString(state), currentDist, bestDir);
        return;
    }

    switch (state) {
        case State::AUTO_FORWARD: {
            motor.forward();
            currentDist = ultrasonic.measureAvg();
            if (currentDist > 0.0f && currentDist < DANGER_DIST_CM) {
                motor.stop();
                Serial.printf("[STATE] AUTO_FORWARD -> AUTO_SCANNING | dist=%.1fcm\n", currentDist);
                state = State::AUTO_SCANNING;
            }
            break;
        }

        case State::AUTO_SCANNING: {
            motor.stop();

            servo.left();
            delay(SERVO_SETTLE_MS);
            float distLeft = ultrasonic.measureAvg();

            servo.center();
            delay(SERVO_SETTLE_MS);
            float distFront = ultrasonic.measureAvg();

            servo.right();
            delay(SERVO_SETTLE_MS);
            float distRight = ultrasonic.measureAvg();

            servo.center();

            float effL = (distLeft < 0.0f) ? 400.0f : distLeft;
            float effF = (distFront < 0.0f) ? 400.0f : distFront;
            float effR = (distRight < 0.0f) ? 400.0f : distRight;

            Serial.printf("[SCAN] L=%.1f F=%.1f R=%.1f\n", effL, effF, effR);

            if (effL < DANGER_DIST_CM && effF < DANGER_DIST_CM && effR < DANGER_DIST_CM) {
                bestDir = "NONE";
                state = State::AUTO_BACKWARD;
                Serial.println("[STATE] AUTO_SCANNING -> AUTO_BACKWARD");
            } else if (effL >= effF && effL >= effR) {
                bestDir = "LEFT";
                state = State::AUTO_TURNING;
                Serial.println("[STATE] AUTO_SCANNING -> AUTO_TURNING LEFT");
            } else if (effR >= effF && effR >= effL) {
                bestDir = "RIGHT";
                state = State::AUTO_TURNING;
                Serial.println("[STATE] AUTO_SCANNING -> AUTO_TURNING RIGHT");
            } else {
                bestDir = "FWD";
                state = State::AUTO_FORWARD;
                Serial.println("[STATE] AUTO_SCANNING -> AUTO_FORWARD (front clear)");
            }
            break;
        }

        case State::AUTO_TURNING: {
            if (strcmp(bestDir, "LEFT") == 0) {
                motor.turnLeft();
            } else {
                motor.turnRight();
            }
            delay(TURN_DURATION_MS);
            motor.stop();
            Serial.printf("[STATE] AUTO_TURNING %s -> AUTO_FORWARD\n", bestDir);
            state = State::AUTO_FORWARD;
            break;
        }

        case State::AUTO_BACKWARD: {
            motor.backward();
            delay(BACK_DURATION_MS);
            motor.stop();
            Serial.println("[STATE] AUTO_BACKWARD -> AUTO_SCANNING");
            state = State::AUTO_SCANNING;
            break;
        }

        default:
            break;
    }

    display.update(stateToString(state), currentDist, bestDir);
}
