#include "OpenGLCamera.h"

void OpenGlCamera::update() {
    double mouseX, mouseY;
    glfwGetCursorPos(mWindow, &mouseX, &mouseY);
    if (mLastX != mouseX || mLastY != mouseY) {
        float xOffset = mouseX - mLastX;
        float yOffset = mouseY - mLastY;
        mLastX = mouseX;
        mLastY = mouseY;
        mPitch -= yOffset * mSensitivity;
        mYaw += xOffset * mSensitivity;
    }
    if (mPitch > 89.0f) {
        mPitch = 89.0f;
    }
    if (mPitch < -89.0f) {
        mPitch = -89.0f;
    }
    Gem::fVec3 front;
    front.x = std::cos(Gem::ToRadians(mYaw)) * std::cos(Gem::ToRadians(mPitch));
    front.y = std::sin(Gem::ToRadians(mPitch));
    front.z = std::sin(Gem::ToRadians(mYaw)) * std::cos(Gem::ToRadians(mPitch));
    mDirection = front.normalize();
    if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) {
        mPosition += mDirection * mSpeed;
    }
    if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) {
        mPosition -= mDirection * mSpeed;
    }
    if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) {
        mPosition -= Gem::Cross(mDirection, mUp).normalize() * mSpeed;
    }
    if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) {
        mPosition += Gem::Cross(mDirection, mUp).normalize() * mSpeed;
    }
    if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
        mPosition -= Gem::fVec3(0.f, -mSpeed, 0.f);
    }
    if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        mPosition -= Gem::fVec3(0.f, mSpeed, 0.f);
    }
}
Gem::fMat4 OpenGlCamera::getViewMatrix() {
    return Gem::LookAt(mPosition, mPosition + mDirection, mUp);
}
Gem::fVec3 OpenGlCamera::getCameraPos() {
    return mPosition;
}
void OpenGlCamera::setCameraPos(Gem::fVec3 position) {
    mPosition = position;
}


