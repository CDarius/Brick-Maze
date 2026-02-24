#include "CancelToken.hpp"

bool CancelToken::isCancelled() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    return cancelled;
}

void CancelToken::cancel() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    cancelled = true;
}

void delayCancellable(unsigned long delayMs, CancelToken& token) {
    unsigned long startTime = millis();
    const unsigned long POLL_INTERVAL_MS = 10; // Check cancel token every 10ms
    
    while (!token.isCancelled()) {
        unsigned long elapsedTime = millis() - startTime;
        if (elapsedTime >= delayMs) {
            break; // Delay duration completed
        }
        
        // Calculate remaining time and delay interval
        unsigned long remainingTime = delayMs - elapsedTime;
        unsigned long sleepDuration = (remainingTime < POLL_INTERVAL_MS) ? remainingTime : POLL_INTERVAL_MS;
        delay(sleepDuration);
    }
}