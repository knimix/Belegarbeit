#include "Shader.h"
#include <fstream>
#include <Nuc/Log.h>

bool Shader::loadStage(std::string_view path, ShaderType type) {
    VkShaderModule module;
    std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    VkShaderModuleCreateInfo createInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
    if (vkCreateShaderModule(mContext->device, &createInfo, nullptr, &module) != VK_SUCCESS) {
        LOG_WARN("Failed to create vulkan-shader module");
        return false;
    }
    VkPipelineShaderStageCreateInfo stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stage.stage = type == ShaderType::Vertex ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
    stage.module = module;
    stage.pName = "main";
    stages.emplace_back(stage);

    return true;
}
void Shader::terminate() {
    for(auto stage : stages){
        vkDestroyShaderModule(mContext->device,stage.module, nullptr);
    }
}
