#include "Swapchain.h"
#include <Nuc/Log.h>

bool Swapchain::initialize(VkSurfaceKHR surface, Ref<VulkanContext>& context) {
    mSurface = surface;
    mContext = context;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, surface, &surfaceCapabilities);
    mMinImageCount = surfaceCapabilities.minImageCount+1;
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, surface, &formatCount, formats.data());
    format = formats[0].format;
    colorSpace = formats[0].colorSpace;
    int debugFormat = format;
    int debugColorSpace = colorSpace;
    LOG_DEBUG("Initialize swapchain, minImageCount: {}, format: {}, colorSpace: {}", mMinImageCount, debugFormat, debugColorSpace);

    VkAttachmentDescription descriptions[2] = {};
    descriptions[0] = {};
    descriptions[0].format = format;
    descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    descriptions[0].flags = 0;

    descriptions[1] = {};
    descriptions[1].format = VK_FORMAT_D32_SFLOAT;
    descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    descriptions[1].flags = 0;

    VkAttachmentReference attachmentReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthAttachmentReference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentReference;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.pInputAttachments = nullptr;
    subpass.pPreserveAttachments = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    VkRenderPassCreateInfo renderPassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.pAttachments = descriptions;
    renderPassCreateInfo.attachmentCount = 2;
    if (vkCreateRenderPass(mContext->device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
        LOG_DEBUG("Failed to create render pass");
        return false;
    }

    return resize();
}
bool Swapchain::resize() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mContext->physicalDevice, mSurface, &surfaceCapabilities);
    if (surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.currentExtent.height == 0) {
        return false;
    }
    width = surfaceCapabilities.currentExtent.width;
    height = surfaceCapabilities.currentExtent.height;
    VkSwapchainKHR oldSwapchian = swapchain;
    VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    createInfo.surface = mSurface;
    createInfo.minImageCount = mMinImageCount;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.imageFormat = format;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageArrayLayers = 1;
    createInfo.imageExtent = surfaceCapabilities.currentExtent;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.oldSwapchain = oldSwapchian;
    if (vkCreateSwapchainKHR(mContext->device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        LOG_WARN("Failed to create swapchain");
        return false;
    }

    vkDestroySwapchainKHR(mContext->device,oldSwapchian, nullptr);

    for(auto imageView : swapchainImageViews){
        vkDestroyImageView(mContext->device, imageView, nullptr);
    }

    //get images from created swapchain

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(mContext->device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mContext->device, swapchain, &imageCount, swapchainImages.data());
    swapchainImageViews.resize(imageCount);
    for (int i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageCreateInfo.image = swapchainImages[i];
        imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY};
        imageCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        if (vkCreateImageView(mContext->device, &imageCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
            LOG_WARN("Failed to create image view");
            return false;
        }
    }
    //create framebuffers;
    for(auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(mContext->device,framebuffer, nullptr);
    }
    for(auto& depthImage : depthImages){
        depthImage->destroy();
    }
    depthImages.clear();
    depthImages.resize(swapchainImages.size());

    framebuffers.resize(swapchainImages.size());
    for (int i = 0; i < framebuffers.size(); i++) {

        auto depthImage = MakeRef<Image>(mContext);
        depthImages[i] = depthImage;

        depthImage->create(width,height,VK_FORMAT_D32_SFLOAT,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        VkImageView attachments[] = {
                swapchainImageViews[i],
                depthImages[i]->imageView
        };

        VkFramebufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = 2;
        createInfo.pAttachments = attachments;
        createInfo.width = surfaceCapabilities.currentExtent.width;
        createInfo.height = surfaceCapabilities.currentExtent.height;
        createInfo.layers = 1;
        if (vkCreateFramebuffer(mContext->device, &createInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            LOG_WARN("Failed to create framebuffer");
            return false;
        }
    }

    return true;
}
void Swapchain::terminate() {
    for(auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(mContext->device,framebuffer, nullptr);
    }
    for(auto& depthImage : depthImages){
        depthImage->destroy();
    }
    vkDestroyRenderPass(mContext->device,renderPass, nullptr);
    for(auto imageView : swapchainImageViews){
        vkDestroyImageView(mContext->device,imageView, nullptr);
    }
    vkDestroySwapchainKHR(mContext->device,swapchain, nullptr);
}
void Swapchain::next(const Gem::fVec4& clearColor) {
    //get next image from swapchain
    auto result = vkAcquireNextImageKHR(mContext->device, swapchain, UINT64_MAX, mContext->acquireSemaphore, nullptr, &mImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        LOG_WARN("Acquire image failed");
    }

    // wait for previous frame to finish
    vkWaitForFences(mContext->device, 1, &mContext->fence, VK_TRUE, UINT64_MAX);
    vkResetFences(mContext->device, 1, &mContext->fence);

    //reset command pool
    vkResetCommandPool(mContext->device,mContext->commandPool,0);

    //begin command buffer;
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(mContext->commandBuffer, &beginInfo);

    //begin renderpass


}
void Swapchain::flush() {

    vkCmdEndRenderPass(mContext->commandBuffer);
    vkEndCommandBuffer(mContext->commandBuffer);

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mContext->commandBuffer;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mContext->acquireSemaphore;
    VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitMask;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mContext->submitSemaphore;
    vkQueueSubmit(mContext->commandQueue, 1, &submitInfo, mContext->fence);


    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &mImageIndex;
    presentInfo.pWaitSemaphores = &mContext->submitSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    vkQueuePresentKHR(mContext->commandQueue, &presentInfo);
}


