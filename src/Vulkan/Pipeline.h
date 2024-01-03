#pragma once
#include "Swapchain.h"
#include "Shader.h"
#include <Nuc/Memory.h>
class Pipeline {
public:
    Pipeline(Ref<Swapchain>& swapchain, Ref<Shader>& shader, VkPipelineVertexInputStateCreateInfo vertexLayout) : mSwapchain(swapchain), mShader(shader), mVertexLayout(vertexLayout){};
    bool build();
    void terminate();
    VkPipeline pipeline = nullptr;
    VkPipelineLayout layout = nullptr;
private:
    Ref<Swapchain> mSwapchain;
    Ref<Shader> mShader;
    VkPipelineVertexInputStateCreateInfo mVertexLayout;

};
