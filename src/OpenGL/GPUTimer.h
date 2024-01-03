#pragma once
#include <chrono>

namespace Engine {
    class GPUTimer {
    public:
        GPUTimer();
        ~GPUTimer();
        explicit GPUTimer(bool autoStart);
        void start();
        void stop();
        void reset();
        template<typename T>
        long time() {
            return std::chrono::duration_cast<T>(std::chrono::nanoseconds(mDuration)).count();
        }
    private:
        uint64_t mDuration = 0;
        uint32_t mQueryID[2]{};
        bool mStarted = false;
    };
}
