#include "GPUTimer.h"
#include <glad/glad.h>

namespace Engine {
    GPUTimer::GPUTimer() {
        glGenQueries(2, mQueryID);
    }
    GPUTimer::~GPUTimer() {
        glDeleteQueries(2, mQueryID);
    }
    GPUTimer::GPUTimer(bool autoStart) : GPUTimer() {
        if (autoStart) {
            start();
        }
    }
    void GPUTimer::start() {
        glQueryCounter(mQueryID[0], GL_TIMESTAMP);
        mStarted = true;
    }
    void GPUTimer::stop() {
        if (mStarted) {
            glQueryCounter(mQueryID[1], GL_TIMESTAMP);
            int stopTimerAvailable = 0;
            while (!stopTimerAvailable) {
                glGetQueryObjectiv(mQueryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
            }
            GLuint64 localStart, localStop;
            glGetQueryObjectui64v(mQueryID[0], GL_QUERY_RESULT, &localStart);
            glGetQueryObjectui64v(mQueryID[1], GL_QUERY_RESULT, &localStop);
            mDuration += (localStop - localStart);
            mStarted = false;
        }
    }
    void GPUTimer::reset() {
        mStarted = false;
        mDuration = 0;
    }
}