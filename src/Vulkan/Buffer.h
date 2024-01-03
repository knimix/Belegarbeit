#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "VulkanContext.h"
#include <Nuc/Memory.h>
class Buffer {
public:
    explicit Buffer(Ref<VulkanContext>& context) : mContext(context){}
    void create(uint64_t size,VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    void destroy();
    void* map();
    void unmap();
    VkBuffer buffer{};
    VmaAllocation allocation{};
private:
    Ref<VulkanContext> mContext;
};
