#pragma once

#include <Controller.hpp>

ControllerConfig getDefaultControllerConfig() {
    ControllerConfig config {
        .maxAngle = 1.0f, // Max angle in range [0, 1]
        .updateRateMs = 100 // Update rate in milliseconds
    };

    return config;
}