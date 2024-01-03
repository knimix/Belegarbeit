#pragma once
#include <vulkan/vulkan.h>
#include "VulkanContext.h"
#include <Nuc/Memory.h>
#include <Gem/Vec4.h>
#include <vector>
#include "Image.h"
class Swapchain {
public:
    Swapchain() = default;
    bool initialize(VkSurfaceKHR surface, Ref<VulkanContext>& context);
    void terminate();
    bool resize();

    void next(const Gem::fVec4& clearColor);
    void flush();

    VkFormat format{};
    VkColorSpaceKHR colorSpace{};
    VkSwapchainKHR swapchain = nullptr;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<Ref<Image>> depthImages;
    int width,height;
    const Ref<VulkanContext>& getContext() const {return mContext;}
    uint32_t mImageIndex;
private:
    VkSurfaceKHR mSurface{};
    Ref<VulkanContext> mContext;
    int mMinImageCount = 0;
};
