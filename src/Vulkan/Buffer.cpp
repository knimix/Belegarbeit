#include "Buffer.h"

void Buffer::create(uint64_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    createInfo.size = size;
    createInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = memoryUsage;
    vmaCreateBuffer(mContext->allocator, &createInfo, &vmaAllocInfo,&buffer,&allocation,nullptr);
}
void Buffer::destroy() {
    vmaDestroyBuffer(mContext->allocator, buffer, allocation);
}
void* Buffer::map() {
    void* data;
    vmaMapMemory(mContext->allocator, allocation, &data);
    return data;
}
void Buffer::unmap() {
    vmaUnmapMemory(mContext->allocator, allocation);
}
