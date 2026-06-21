#pragma once

#include <stdint.h>

class WiFiControl {
public:
    WiFiControl();
    void begin(const char* ssid, const char* password);

    enum class Command {
        NONE,
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        STOP,
        AUTO
    };

    Command getCommand();
    void setCommand(Command cmd);
    void clearCommand();
    void handle();

private:
    Command currentCommand;
};
