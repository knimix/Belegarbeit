#include "Sampler.h"

void Sampler::create() {
    VkSamplerCreateInfo createInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    createInfo.magFilter = VK_FILTER_NEAREST;
    createInfo.minFilter = VK_FILTER_NEAREST;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.mipLodBias = 0;
    createInfo.maxAnisotropy = 1;
    createInfo.minLod = 0;
    createInfo.maxLod = 1;
    vkCreateSampler(mContext->device,&createInfo, nullptr,&sampler);
}
void Sampler::destroy() {
    vkDestroySampler(mContext->device,sampler, nullptr);
}
