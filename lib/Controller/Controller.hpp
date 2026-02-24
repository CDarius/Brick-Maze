#pragma once

#include <Arduino.h>
#include <SerialComm.hpp>
#include <SerialCommandReader.hpp>

struct ControllerConfig {
    float maxAngle;         // Maximum angle in range [0, 1]
    uint16_t updateRateMs;  // Update rate in milliseconds
};

/**
 * Controller class manages the state and communication of the remote controller. It handles receiving 
 * updates from the controller, sending parameters to the host, and providing the current status 
 * when requested. The update method should be called in a task to process incoming data from the controller.
 */

class Controller {
public:
    /**
     * Constructor for the Controller class.
     * @param serialComm Reference to the SerialComm instance for communication.
     */
    Controller(SerialComm& serialComm) : serialComm(serialComm) {}

    /**
     * Initializes the controller by sending parameters to the host and waiting for communication to be established.
     * @param config The configuration parameters for the controller.
     * @return True if initialization was successful, false otherwise.
     */
    bool begin(ControllerConfig config);

    /**
     * Gets the current maximum angle parameter.
     * @return The maximum angle in range [0, 1].
     */
    float getMaxAngle() const {
        return maxAngle;
    }

    /**
     * Sets the maximum angle parameter.
     * @param angle The maximum angle in range [0, 1].
     */
    void setMaxAngle(float angle) {
        maxAngle = angle;
        serialComm.sendControllerParams(maxAngle, updateRateMs);
    }

    /**
     * Gets the current update rate in milliseconds.
     * The update rate is sent to the controller and the controller is expected to send updates 
     * at approximately this rate. The controller will be considered unresponsive if no updates 
     * are received for more than 2 times this duration.
     * @return The update rate in milliseconds.
     */
    uint16_t getUpdateRate() const {
        return updateRateMs;
    }

    /**
     * Sets the update rate in milliseconds.
     * @param rateMs The update rate in milliseconds.
     */
    void setUpdateRate(uint16_t rateMs) {
        updateRateMs = rateMs;
        serialComm.sendControllerParams(maxAngle, updateRateMs);
    }

    /**
     * Gets the current enabled status of the controller.
     * When not enabled, the controller will not send any data to the host, but it will still read 
     * incoming commands and update its internal state accordingly.
     * @return True if the controller is enabled, false otherwise.
     */
    bool getIsEnabled() const {
        return isEnabled;
    }

    /**
     * Sets the enabled status of the controller.
     * @param enabled True to enable the controller, false to disable it.
     */
    void setIsEnabled(bool enabled) {
        serialComm.sendControllerEnabled(enabled);
        isEnabled = enabled;
    }

    /**
     * Gets the current status of the controller.
     * @param x Reference to store the current x angle.
     * @param y Reference to store the current y angle.
     * @param buttonPressed Reference to store the current button pressed status.
     * @return True if the controller is enabled and the communication is active (i.e., updates have been received within the expected time frame), false otherwise.
     */
    bool getStatus(float& x, float& y, bool& buttonPressed);

    /**
     * Updates the controller's internal state by processing incoming data from the controller.
     * This method should be called in a task to ensure the controller's status is up-to-date.
     */
    void update();

private:
    SerialComm& serialComm;

    float xAngle = 0.0f;
    float yAngle = 0.0f;
    bool isButtonPressed = false;
    unsigned long lastUpdateTime = 0;
    
    float maxAngle;
    uint16_t updateRateMs;
    bool isEnabled = false;

    void updateStatus(float xAngle, float yAngle, bool isButtonPressed);    
};