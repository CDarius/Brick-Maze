#pragma once

#include <Arduino.h>
#include <SerialCommand.hpp>

/**
 * SerialComm class is responsible for handling serial communication with the host. It provides methods to send formatted
 * data (like XY angles) to the host and to read incoming commands from the serial buffer. The readCommands method is
 * non-blocking and processes complete lines of input, parsing them into command and value components.
 */
class SerialComm {
    public:
        /**
         * Constructor for SerialComm.
         * @param serial Reference to the HardwareSerial object to use for communication.
         */
        SerialComm(HardwareSerial& serial) : serial(serial), receiveBuffer("") {}

        /**
         * Sends the current controller parameters to the host.
         * @param maxAcc The maximum acceleration value.
         * @param updateRate The update rate in milliseconds.
         */
        void sendControllerParams(float maxAcc, int updateRate) {
            // Format the string as "SET_CTRL_PARAMS:<maxAcc value>,<updateRate value>\n"
            serial.printf("SET_CTRL_PARAMS:%.3f##%d\n", maxAcc, updateRate);
        }

        /**
         * Sends the current controller enabled state to the host.
         * When not enabled, the controller will not send any data to the host, but 
         * it will still read incoming commands and update its internal state accordingly.
         * @param enabled True if the controller is enabled, false otherwise.
         */
        void sendControllerEnabled(bool enabled) {
            // Format the string as "ENAB_CTRL:<enabled value>\n"
            serial.printf("ENAB_CTRL:%d\n", enabled);
        }

        /**
         * Reads commands from the serial buffer in a non-blocking manner.
         * @return A SerialCommand struct containing the parsed command and its values.
         */
        SerialCommand readCommands();
        
    private:
        HardwareSerial& serial;
        String receiveBuffer;  // Buffer to accumulate incoming data until newline
};