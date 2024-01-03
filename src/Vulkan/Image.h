#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "VulkanContext.h"
#include <Nuc/Memory.h>
class Image {
public:
    explicit Image(Ref<VulkanContext>& context) : mContext(context){}
    bool create(int width, int height, VkFormat format, VkImageUsageFlags usage);
    void destroy();
    void uploadData(uint8_t* data, uint64_t size, VkImageLayout finalLayout, VkAccessFlags destinationMask);
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
private:
    int mWidth, mHeight;
    Ref<VulkanContext> mContext;
};
