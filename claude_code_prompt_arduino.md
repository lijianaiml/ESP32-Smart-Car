# ESP32 Arduino 智能避障小车 —— 完整 PlatformIO 工程

## 任务说明
用 VSCode + PlatformIO + Arduino 框架，为 ESP32 实现一个完整的超声波避障小车工程。
代码必须能直接通过 PlatformIO 编译并烧录，不允许使用任何需要手动安装的第三方库（除下方明确列出的）。

---

## 工程初始化配置

### platformio.ini（必须完全按此生成）
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    adafruit/Adafruit SSD1306 @ ^2.5.7
    adafruit/Adafruit GFX Library @ ^1.11.5
build_flags =
    -DCORE_DEBUG_LEVEL=0
```

---

## 硬件接线（严格按此定义，禁止修改引脚）

```
【电源】
7.2V 电池正极 ──→ LM2596 IN+
7.2V 电池正极 ──→ TB6612 VM（电机电源）
7.2V 电池负极 ──→ LM2596 IN-（所有 GND 共地）
LM2596 OUT+ 5V ──→ ESP32 VIN
LM2596 OUT+ 5V ──→ TB6612 VCC
LM2596 OUT+ 5V ──→ HC-SR04 VCC
LM2596 OUT+ 5V ──→ SG90 VCC（红线）
LM2596 GND ────→ TB6612 GND / HC-SR04 GND / SG90 GND（棕线）/ OLED GND / ESP32 GND
ESP32 3.3V ────→ TB6612 STBY（硬件接高，始终使能）

【TB6612FNG 电机驱动】
A 路 = 左侧两个 TT 电机并联
  AIN1  ← GPIO 26
  AIN2  ← GPIO 27
  PWMA  ← GPIO 25

B 路 = 右侧两个 TT 电机并联
  BIN1  ← GPIO 32
  BIN2  ← GPIO 14
  PWMB  ← GPIO 33

【HC-SR04 超声波】
  VCC  → LM2596 5V
  GND  → 共地
  TRIG ← GPIO 18
  ECHO → GPIO 19  （ECHO 输出 5V，已做分压：ECHO─1kΩ─GPIO19，GPIO19─2kΩ─GND）

【SG90 舵机（搭载超声波传感器）】
  红线（VCC） → LM2596 5V
  棕线（GND） → 共地
  橙线（PWM） ← GPIO 13

【OLED 0.96" I2C SSD1306】
  VCC → LM2596 3.3V 或 ESP32 3.3V
  GND → 共地
  SCL ← GPIO 22
  SDA ← GPIO 21
  I2C 地址：0x3C，分辨率 128×64
```

---

## 文件结构（严格按此创建）

```
project/
├── platformio.ini
└── src/
    ├── main.cpp
    ├── config.h
    ├── Motor.h / Motor.cpp
    ├── ServoCtrl.h / ServoCtrl.cpp
    ├── Ultrasonic.h / Ultrasonic.cpp
    └── OledDisplay.h / OledDisplay.cpp
```

---

## 各文件实现要求

### config.h
用 `#pragma once` + `#define` 或 `constexpr` 定义所有常量，禁止在其他文件出现魔法数字：

```cpp
// 引脚
AIN1=26, AIN2=27, PWMA=25
BIN1=32, BIN2=14, PWMB=33
TRIG_PIN=18, ECHO_PIN=19
SERVO_PIN=13
OLED_SDA=21, OLED_SCL=22

// 电机参数
MOTOR_PWM_FREQ    = 1000   // Hz
MOTOR_PWM_RES     = 8      // bits（0-255）
DEFAULT_SPEED     = 180    // 0-255
TURN_SPEED        = 160
TURN_DURATION_MS  = 550    // 转向持续时间

// 超声波
DANGER_DIST_CM    = 20     // 触发避障距离
MEASURE_TIMEOUT   = 25000  // pulseIn 超时 us（约 4m）
MEASURE_AVG_N     = 3      // 平均测量次数

// 舵机角度（SG90：0°-180°）
SERVO_LEFT        = 160
SERVO_CENTER      = 90
SERVO_RIGHT       = 20
SERVO_SETTLE_MS   = 350    // 舵机到位 + 超声波稳定等待

// 主循环
LOOP_INTERVAL_MS  = 50
BACK_DURATION_MS  = 600    // 三向堵死后退时间
```

---

### Motor.h / Motor.cpp
使用 ESP32 Arduino 的 `ledcSetup` / `ledcAttachPin` / `ledcWrite` 实现 PWM（不使用 analogWrite）。

**Motor 类接口：**
```cpp
class Motor {
public:
    Motor();
    void begin();
    void forward(uint8_t speed = DEFAULT_SPEED);
    void backward(uint8_t speed = DEFAULT_SPEED);
    void turnLeft(uint8_t speed = TURN_SPEED);   // 左轮反转，右轮正转
    void turnRight(uint8_t speed = TURN_SPEED);  // 右轮反转，左轮正转
    void stop();
private:
    void setA(int dir, uint8_t speed);  // dir: 1=正转 -1=反转 0=停
    void setB(int dir, uint8_t speed);
};
```

**PWM 通道分配：**
- PWMA → ledc channel 0
- PWMB → ledc channel 1

**方向逻辑：**
- 正转：IN1=HIGH, IN2=LOW
- 反转：IN1=LOW,  IN2=HIGH
- 刹车：IN1=LOW,  IN2=LOW，speed=0

---

### ServoCtrl.h / ServoCtrl.cpp
使用 Arduino 内置 `ESP32Servo` 或直接用 `ledcWrite` 控制舵机（ledc channel 2，频率 50Hz，分辨率 16bit）。

**优先使用 ledc 方式**（避免引入额外库），脉宽计算：
```
// 50Hz 周期 = 20ms，16bit 分辨率 = 65535
// 0°  → 脉宽 0.5ms → duty = 0.5/20 * 65535 ≈ 1638
// 90° → 脉宽 1.45ms → duty ≈ 4751
// 180°→ 脉宽 2.4ms  → duty ≈ 7864
uint32_t angleToDuty(int angle);  // 线性插值 0~180° → 1638~7864
```

**ServoCtrl 类接口：**
```cpp
class ServoCtrl {
public:
    ServoCtrl();
    void begin();
    void setAngle(int angle);   // 0-180
    void center();              // 90°
    void left();                // SERVO_LEFT
    void right();               // SERVO_RIGHT
};
```

---

### Ultrasonic.h / Ultrasonic.cpp
使用 Arduino `pulseIn()` 实现。

**Ultrasonic 类接口：**
```cpp
class Ultrasonic {
public:
    Ultrasonic();
    void begin();
    float measureCm();       // 单次测量，超时返回 -1.0f
    float measureAvg();      // 测 MEASURE_AVG_N 次，去最大值取平均，全部超时返回 -1.0f
private:
    float rawMeasure();
};
```

**measureCm 实现要点：**
1. TRIG 低→高(10µs)→低
2. `pulseIn(ECHO_PIN, HIGH, MEASURE_TIMEOUT)` 获取脉宽 us
3. 返回 0 表示超时，distance = duration * 0.0343f / 2.0f
4. distance > 400 或 distance <= 0 → 返回 -1.0f

---

### OledDisplay.h / OledDisplay.cpp
使用 `Adafruit_SSD1306`，Wire 初始化时指定 SDA=21 SCL=22。

**OledDisplay 类接口：**
```cpp
class OledDisplay {
public:
    OledDisplay();
    void begin();
    void showBoot();           // 开机画面，显示 1.5s
    void update(const char* state, float dist, const char* bestDir);
    // 显示布局：
    // Line1（y=0）:  "State: FORWARD"
    // Line2（y=20）: "Dist:  XX.X cm"  （-1 显示 "Dist: --- cm"）
    // Line3（y=40）: "Best:  LEFT"
};
```

---

### main.cpp
实现状态机主循环，使用 `enum class State` 定义状态：

```cpp
enum class State {
    FORWARD,    // 前进，持续测距
    SCANNING,   // 停车，舵机扫描三方向
    TURNING,    // 执行转向
    BACKWARD    // 三向全堵，后退
};
```

**主循环完整逻辑：**

```
setup():
  Serial.begin(115200)
  motor.begin()
  servo.begin() → servo.center()
  ultrasonic.begin()
  display.begin() → display.showBoot()
  state = FORWARD
  bestDir = "FWD"
  delay(500)

loop():
  static uint32_t lastLoop = 0
  if (millis() - lastLoop < LOOP_INTERVAL_MS) return
  lastLoop = millis()

  switch(state):

    case FORWARD:
      motor.forward()
      dist = ultrasonic.measureAvg()
      if dist > 0 && dist < DANGER_DIST_CM:
        motor.stop()
        Serial.printf("[STATE] FORWARD → SCANNING | dist=%.1fcm\n", dist)
        state = SCANNING

    case SCANNING:
      motor.stop()

      servo.left()
      delay(SERVO_SETTLE_MS)
      distLeft = ultrasonic.measureAvg()

      servo.center()
      delay(SERVO_SETTLE_MS)
      distFront = ultrasonic.measureAvg()

      servo.right()
      delay(SERVO_SETTLE_MS)
      distRight = ultrasonic.measureAvg()

      servo.center()

      // -1（超时/无障碍）视为 400cm
      float effL = (distLeft  < 0) ? 400.0f : distLeft
      float effF = (distFront < 0) ? 400.0f : distFront
      float effR = (distRight < 0) ? 400.0f : distRight

      Serial.printf("[SCAN] L=%.1f F=%.1f R=%.1f\n", effL, effF, effR)

      if (effL < DANGER_DIST_CM && effF < DANGER_DIST_CM && effR < DANGER_DIST_CM):
        bestDir = "NONE"
        state = BACKWARD
        Serial.println("[STATE] SCANNING → BACKWARD")
      else if (effL >= effF && effL >= effR):
        bestDir = "LEFT"
        state = TURNING
        Serial.println("[STATE] SCANNING → TURNING LEFT")
      else if (effR >= effF && effR >= effL):
        bestDir = "RIGHT"
        state = TURNING
        Serial.println("[STATE] SCANNING → TURNING RIGHT")
      else:
        bestDir = "FWD"
        state = FORWARD
        Serial.println("[STATE] SCANNING → FORWARD (front clear)")

    case TURNING:
      if (bestDir == "LEFT"):
        motor.turnLeft()
      else:
        motor.turnRight()
      delay(TURN_DURATION_MS)
      motor.stop()
      Serial.printf("[STATE] TURNING %s → FORWARD\n", bestDir)
      state = FORWARD

    case BACKWARD:
      motor.backward()
      delay(BACK_DURATION_MS)
      motor.stop()
      Serial.println("[STATE] BACKWARD → SCANNING")
      state = SCANNING

  // 每轮更新 OLED（传入当前状态字符串、前方最近距离、bestDir）
  display.update(stateStr, dist, bestDir)
```

---

## 代码质量强制要求

1. **编译零警告**：所有变量必须初始化，不允许隐式类型转换警告
2. **ledc 通道不冲突**：Motor 用 ch0/ch1，Servo 用 ch2，三者频率和分辨率分开设置
3. **Serial 日志**：每次状态切换必须打印，格式统一为 `[STATE] FROM → TO | 附加信息`
4. **头文件保护**：所有 .h 文件使用 `#pragma once`
5. **include 顺序**：系统库 → 第三方库 → 本地头文件
6. **stop() 实现**：ledc duty 置 0 + 方向引脚全低（刹车模式，非滑行）
7. **Wire 初始化**：`Wire.begin(OLED_SDA, OLED_SCL)` 必须在 `Adafruit_SSD1306` 构造前调用
8. **不使用 delay() 在主状态机轮询中**：仅在 SCANNING 舵机等待、TURNING、BACKWARD 三处允许使用 delay()

---

## 交付检查清单

- [ ] platformio.ini 配置正确，lib_deps 只用 Adafruit SSD1306 + GFX
- [ ] config.h 包含全部引脚和参数，无魔法数字散落在其他文件
- [ ] Motor.cpp 用 ledcSetup/ledcAttachPin/ledcWrite，ch0 ch1，PWM 8bit 1000Hz
- [ ] ServoCtrl.cpp 用 ledcSetup ch2，50Hz，16bit，angleToDuty 计算正确
- [ ] Ultrasonic.cpp 含超时保护，measureAvg 去最大值取均值
- [ ] OledDisplay.cpp Wire.begin 指定 SDA/SCL，三行显示正确
- [ ] main.cpp 状态机四个状态完整，逻辑闭环，无死循环
- [ ] 全工程 pio run 编译通过，零错误零警告
