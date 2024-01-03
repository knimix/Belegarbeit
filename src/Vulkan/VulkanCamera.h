#pragma once
#include<GLFW/glfw3.h>
#include <Gem/Vec3.h>
#include <Gem/Mat4.h>

class VulkanCamera {
public:
    VulkanCamera(GLFWwindow* window) : mWindow(window) {}
    void update();
    Gem::fMat4 getViewMatrix();
    Gem::fVec3 getCameraPos();;
    void setCameraPos(Gem::fVec3 position);
private:
    GLFWwindow* mWindow;
    Gem::fVec3 mPosition = {};
    Gem::fVec3 mDirection = {};
    Gem::fVec3 mUp = {0.0f, 1.0f, 0.0f};
    float mYaw = 0,mPitch = 0;
    float mSpeed = 0.1, mSensitivity = 0.1;
    double mLastX, mLastY;
};


