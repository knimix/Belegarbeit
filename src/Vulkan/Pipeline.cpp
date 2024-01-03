#include "Pipeline.h"
#include <Nuc/Log.h>
#include <Gem/Mat4.h>

bool Pipeline::build() {
    auto& context = mSwapchain->getContext();
    if (layout != nullptr) {
        vkDestroyPipelineLayout(context->device, layout, nullptr);
    }
    if (pipeline != nullptr) {
        vkDestroyPipeline(context->device, pipeline, nullptr);
    }

    //create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &context->descriptorSetLayout;
    VkPushConstantRange pushConstantRange;

    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(Gem::fMat4);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(context->device, &pipelineLayoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
        LOG_WARN("Failed to create pipeline layout");
        return false;
    }

    //draw mode
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    //viewport
    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    VkViewport viewport = {0, 0, (float)mSwapchain->width, (float)mSwapchain->height,0,1.0};
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    VkRect2D scissor = {{0, 0}, {(uint32_t)mSwapchain->width, (uint32_t)mSwapchain->height}};
    viewportState.pScissors = &scissor;

    //rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationState = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;


    //multisampling

    VkPipelineMultisampleStateCreateInfo multisampleState = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.minSampleShading = 1.0;
    multisampleState.pSampleMask = nullptr;
    multisampleState.alphaToCoverageEnable = VK_FALSE;
    multisampleState.alphaToOneEnable = VK_FALSE;

    //blending state
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    //blend mode
    VkPipelineColorBlendStateCreateInfo colorBlendState = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachment;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.logicOpEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
    depthStencilState.minDepthBounds = 0;
    depthStencilState.maxDepthBounds = 0.0;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.compareOp = VK_COMPARE_OP_LESS;



    VkGraphicsPipelineCreateInfo createInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    createInfo.stageCount = mShader->stages.size();
    createInfo.pStages = mShader->stages.data();
    createInfo.pVertexInputState = &mVertexLayout;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pDepthStencilState = &depthStencilState;
    createInfo.layout = layout;
    createInfo.renderPass = mSwapchain->renderPass;
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(context->device, nullptr, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
        LOG_WARN("Failed to create pipeline");
        return false;
    }
    return true;
}
void Pipeline::terminate() {
    vkDestroyPipelineLayout(mSwapchain->getContext()->device, layout, nullptr);
    vkDestroyPipeline(mSwapchain->getContext()->device, pipeline, nullptr);
}
