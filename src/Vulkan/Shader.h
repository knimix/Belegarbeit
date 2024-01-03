#pragma once
#include "VulkanContext.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string_view>
#include <Nuc/Memory.h>

enum class ShaderType{
    Vertex, Fragment
};

class Shader {
public:
    explicit Shader(Ref<VulkanContext>& context) : mContext(context){};
    void terminate();
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    bool loadStage(std::string_view path, ShaderType type);

private:
    Ref<VulkanContext> mContext;
};
