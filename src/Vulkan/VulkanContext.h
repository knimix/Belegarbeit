#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
class VulkanContext {
public:
    VulkanContext() = default;
    bool initialize(bool validationLayer);
    void terminate() const;
    VkInstance instance{};
    VkPhysicalDevice physicalDevice{};
    VkDevice device{};
    VkQueue commandQueue{};
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkFence fence;
    VkSemaphore acquireSemaphore;
    VkSemaphore submitSemaphore;
    VmaAllocator allocator;
    VkDebugUtilsMessengerEXT debugger;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout  descriptorSetLayout;
private:
    uint32_t mCommandQueueFamilyIndex = -1;
};
