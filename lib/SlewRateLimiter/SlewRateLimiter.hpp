#pragma once
#include <cmath>
#include <algorithm>

/**
 * @brief Generic value interpolator with maximum rate of change constraint.
 * 
 * This class smoothly interpolates a value from its current state to a target state,
 * limiting the rate of change to avoid sudden jumps. Useful for servo control,
 * motor speed ramps, and other smooth transitions.
 * 
 * @tparam T Numeric type (e.g., float, int, double)
 * 
 * Example usage:
 * @code
 * SlewRateLimiter<float> pulseRamper(1500.0f, 500.0f);  // Initial: 1500us, Max rate: 500us/sec
 * pulseRamper.setTarget(2000.0f);
 * 
 * while (!pulseRamper.isAtTarget()) {
 *     pulseRamper.update(20);  // 20ms delta time
 *     servo.writeMicroseconds(pulseRamper.getCurrentValue());
 * }
 * @endcode
 */
template <typename T>
class SlewRateLimiter {
private:
    T currentValue;      ///< Current interpolated value
    T targetValue;       ///< Target value to reach
    float maxRateMs;     ///< Maximum rate of change per millisecond (units/ms)

public:
    /**
     * @brief Constructor
     * @param initialValue Starting value (default: 0)
     * @param maxRatePerSecond Maximum rate of change per second (default: 1.0 units/sec)
     */
    SlewRateLimiter(T initialValue = 0, T maxRatePerSecond = 1.0)
    {
        reset(initialValue);
        setMaxRate(maxRatePerSecond);
    }

    /**
     * @brief Set the target value to interpolate towards
     * @param target The desired target value
     */
    void setTarget(T target) {
        targetValue = target;
    }

    /**
     * @brief Update the current value based on elapsed time
     * 
     * Call this regularly (e.g., in a loop or timer) with the time delta since last update.
     * 
     * @param deltaTimeMs Time elapsed since last update in milliseconds
     */
    void update(uint32_t deltaTimeMs) {
        T delta = targetValue - currentValue;
        T maxChange = static_cast<T>(maxRateMs * deltaTimeMs);

        if (std::abs(delta) <= maxChange) {
            // Close enough, snap to target
            currentValue = targetValue;
        } else {
            // Apply rate limit
            if (delta > 0) {
                currentValue += maxChange;
            } else {
                currentValue -= maxChange;
            }
        }
    }

    /**
     * @brief Get the current interpolated value
     * @return The current value
     */
    T getCurrentValue() const {
        return currentValue;
    }

    /**
     * @brief Get the target value
     * @return The target value
     */
    T getTargetValue() const {
        return targetValue;
    }

    /**
     * @brief Check if the current value has reached the target
     * @return true if current value equals target, false otherwise
     */
    bool isAtTarget() const {
        return currentValue == targetValue;
    }

    /**
     * @brief Set the maximum rate of change
     * @param ratePerSecond The new maximum rate in units per second
     */
    void setMaxRate(T ratePerSecond) {
        maxRateMs = static_cast<float>(ratePerSecond / 1000.0f);
    }

    /**
     * @brief Get the maximum rate of change
     * @return Maximum rate in units per second
     */
    T getMaxRate() const {
        return maxRateMs * 1000.0;
    }

    /**
     * @brief Reset to a new initial state
     * @param value The new current and target value
     */
    void reset(T value) {
        currentValue = value;
        targetValue = value;
    }

    /**
     * @brief Get the distance to target
     * @return Absolute difference between current and target value
     */
    T getDistanceToTarget() const {
        return std::abs(targetValue - currentValue);
    }
};
