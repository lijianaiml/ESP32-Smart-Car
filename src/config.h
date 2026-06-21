#pragma once

// ============ 引脚定义 ============
constexpr uint8_t PIN_AIN1 = 26;
constexpr uint8_t PIN_AIN2 = 27;
constexpr uint8_t PIN_PWMA = 25;

constexpr uint8_t PIN_BIN1 = 32;
constexpr uint8_t PIN_BIN2 = 14;
constexpr uint8_t PIN_PWMB = 33;

constexpr uint8_t PIN_TRIG = 18;
constexpr uint8_t PIN_ECHO = 19;

constexpr uint8_t PIN_SERVO = 13;

constexpr uint8_t PIN_OLED_SDA = 21;
constexpr uint8_t PIN_OLED_SCL = 22;

// ============ 电机参数 ============
constexpr uint32_t MOTOR_PWM_FREQ = 1000;
constexpr uint8_t MOTOR_PWM_RES = 8;
constexpr uint8_t DEFAULT_SPEED = 180;
constexpr uint8_t TURN_SPEED = 160;
constexpr uint32_t TURN_DURATION_MS = 550;

// ============ 超声波参数 ============
constexpr float DANGER_DIST_CM = 20.0f;
constexpr uint32_t MEASURE_TIMEOUT = 25000;
constexpr uint8_t MEASURE_AVG_N = 3;

// ============ 舵机参数 ============
constexpr int SERVO_LEFT = 160;
constexpr int SERVO_CENTER = 90;
constexpr int SERVO_RIGHT = 20;
constexpr uint32_t SERVO_SETTLE_MS = 350;

// ============ 主循环参数 ============
constexpr uint32_t LOOP_INTERVAL_MS = 50;
constexpr uint32_t BACK_DURATION_MS = 600;
