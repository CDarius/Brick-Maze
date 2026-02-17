#include "CancelToken.hpp"

bool CancelToken::isCancelled() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    return cancelled;
}

void CancelToken::cancel() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    cancelled = true;
}