#include "Controller.hpp"

bool Controller::begin(ControllerConfig config) {
    isEnabled = true;
    updateRateMs = config.updateRateMs;
    maxAngle = config.maxAngle;

    // Update the controller parameters and wait for serial communication activity
    uint8_t retry = 10;
    while (retry > 0) {
        serialComm.sendControllerParams(maxAngle, updateRateMs);
        serialComm.sendControllerEnabled(isEnabled);
        delay(updateRateMs * 2); // Wait for a couple of update cycles
        SerialCommand cmd = serialComm.readCommands();
        if (cmd.isValid && cmd.command == "DATA") {
            break; // Communication established
        }
        retry--;

        if (retry == 0) {
            return false;
        }
    }

    // Send initial parameters to ensure host is aware of the current settings
    serialComm.sendControllerParams(maxAngle, updateRateMs);
    serialComm.sendControllerEnabled(isEnabled);

    return true;
}


void Controller::updateStatus(float xAngle, float yAngle, bool isButtonPressed) {
    // Store the new status
    this->xAngle = xAngle;
    this->yAngle = yAngle;
    this->isButtonPressed = isButtonPressed;
    lastUpdateTime = millis();    
}

bool Controller::getStatus(float& x, float& y, bool& buttonPressed) {
    // Return false if controller is disabled
    if (!isEnabled) {
        return false;
    }
    
    // Check if more than 2 times the update rate has elapsed since last updateStatus
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastUpdateTime;
    unsigned long maxElapsedTime = 2 * updateRateMs;
    
    if (elapsedTime > maxElapsedTime) {
        return false;
    }
    
    // Return current status via reference parameters
    x = xAngle;
    y = yAngle;
    buttonPressed = isButtonPressed;
    
    return true;
}

void Controller::update() {
    while (true) {
        SerialCommand cmd = serialComm.readCommands();
        while (cmd.isValid) {
            if (cmd.command == "DATA") {
                // Expected format: "DATA:<x value>,<y value>,<button state>"
                SerialCommandReader reader(cmd);
                float xVal, yVal;
                bool buttonVal;
                if (reader.getFloat(xVal) && reader.getFloat(yVal) && reader.getBool(buttonVal)) {
                    updateStatus(xVal, yVal, buttonVal);
                } else {
                    SerialCommandReader reader(cmd);
                    Serial.println("X value valid: " + String(reader.getFloat(xVal)));
                    Serial.println("Y value valid: " + String(reader.getFloat(yVal)));
                    Serial.println("Button value valid: " + String(reader.getBool(buttonVal)));
                    Serial.println("Invalid DATA command format: " + cmd.values);
                }
            } else {
                Serial.println("Unknown command received: " + cmd.command);
            }
            cmd = serialComm.readCommands();            
        }

        delay(updateRateMs >> 4); // Small delay to prevent busy looping
    }
}
