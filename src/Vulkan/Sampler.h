#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "VulkanContext.h"
#include <Nuc/Memory.h>
class Sampler {
public:
    explicit Sampler(Ref<VulkanContext>& context) : mContext(context){}
    void create();
    void destroy();
    VkSampler sampler;
private:
    Ref<VulkanContext> mContext;
};
