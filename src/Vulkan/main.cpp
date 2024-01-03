#include <Nuc/Log.h>
#include <Nuc/Memory.h>
#include "VulkanContext.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include <GLFW/glfw3.h>
#include <Nuc/Timer.h>
#include "VulkanCamera.h"
#include "Buffer.h"
#include "Image.h"
#include "Sampler.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Nuc/Random.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

int gObjectCount = 100000;
bool gUpdate = false;

int main(int argc, char* argv[]) {
    if(argc == 3){
        gObjectCount = std::stoi(argv[1]);
        gUpdate = std::stoi(argv[2]);
    }
  
    glfwInit();

    LOG_INFO("Belegarbeit Experiment - Vulkan Renderer");
    LOG_DEBUG("ObjectCount: {}   Updating: {}", gObjectCount,gUpdate);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto window = glfwCreateWindow(800,600,"Belegarbeit Experiment - Vulkan Renderer", nullptr, nullptr);
 //   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);

    //creating vulkan context
    auto context = MakeRef<VulkanContext>();
    if(!context->initialize(false)){
        LOG_ERROR("Failed to initialize vulkan context");
        exit(-1);
    }

    //creating window surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(context->instance, window, nullptr, &surface) != VK_SUCCESS) {
        LOG_ERROR("Failed to create     window surface");
        exit(-1);
    }

    //VulkanCamera
    VulkanCamera camera(window);
    auto perspective = Gem::PerspectiveInverseZ(90.f,800.f,600.f,0.01f);
    //creating swapchain

    camera.setCameraPos({30,0,0});

    auto swapchain = MakeRef<Swapchain>();
    if(!swapchain->initialize(surface,context)){
        LOG_ERROR("Failed to initialize swapchain");
        exit(-1);
    }
    auto pushDesc = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(context->device, "vkCmdPushDescriptorSetKHR");

    int imageWidth, imageHeight, channels;
    auto imageData = stbi_load("../assets/discord.JPG",&imageWidth,&imageHeight,&channels,4);
    auto image = MakeRef<Image>(context);
    if(!image->create(imageWidth,imageHeight,VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)){
        LOG_WARN("Failed to create image");
    }
    image->uploadData(imageData,imageHeight * imageWidth * 4,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    stbi_image_free(imageData);

    auto sampler = MakeRef<Sampler>(context);
    sampler->create();


    VkQueryPool gpuTimeQueryPool;

    VkQueryPoolCreateInfo queryPoolCreateInfo{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolCreateInfo.queryCount = 64;
    vkCreateQueryPool(context->device,&queryPoolCreateInfo, nullptr,&gpuTimeQueryPool);

    const int objectCount = gObjectCount;
    struct ObjectLayout{
        Gem::fMat4 model;
        Gem::fVec3  color;
        uint16_t padding;
    };
    LOG_DEBUG("Size: {}", sizeof(ObjectLayout));
    int maxRange = 25;
    int minRage = -25;

    auto objectBuffer = MakeRef<Buffer>(context);
    objectBuffer->create(objectCount * sizeof(ObjectLayout),VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto bufferData = (ObjectLayout*)objectBuffer->map();
    Nuc::Random random;
    for(int i = 0; i< objectCount;i++){
        bufferData[i].model = Gem::TranslationMatrix({random.next<float>(minRage,maxRange),random.next<float>(minRage,maxRange),random.next<float>(minRage,maxRange)});
        bufferData[i].color = {random.next<float>(0.0,1.0),random.next<float>(0.0,1.0),random.next<float>(0.0,1.0)};
    }
    objectBuffer->unmap();


    VkDescriptorImageInfo imageInfo = {sampler->sampler, image->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};



    VkWriteDescriptorSet descriptorWrites[2];
    descriptorWrites[0] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrites[0].dstSet = context->descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].pImageInfo = &imageInfo;

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = objectBuffer->buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = objectCount* sizeof(ObjectLayout);

    descriptorWrites[1] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrites[1].dstSet = context->descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(context->device,2,descriptorWrites,0, nullptr);

    auto shader= MakeRef<Shader>(context);
    shader->loadStage("../assets/vulkan-shader/vertex.spv",ShaderType::Vertex);
    shader->loadStage("../assets/vulkan-shader/fragment.spv",ShaderType::Fragment);


    VkVertexInputAttributeDescription vertexInputDescription[2] = {};
    vertexInputDescription[0].binding = 0;
    vertexInputDescription[0].location = 0;
    vertexInputDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputDescription[0].offset = 0;

    vertexInputDescription[1].binding = 0;
    vertexInputDescription[1].location = 1;
    vertexInputDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputDescription[1].offset = sizeof(float)*3;

    VkVertexInputBindingDescription vertexInputBindingDescription = {};
    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBindingDescription.stride = sizeof(float) * 5;
    VkPipelineVertexInputStateCreateInfo vertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &vertexInputBindingDescription;
    vertexInputState.vertexAttributeDescriptionCount = 2;
    vertexInputState.pVertexAttributeDescriptions = vertexInputDescription;


    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(context->physicalDevice,&properties);

    float data[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // A 0
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // B 1
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // C 2
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // D 3
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // E 4
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   // F 5
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   // G 6
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   // H 7

            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,  // D 8
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // A 9
            -0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  // E 10
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  // H 11
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   // B 12
            0.5f,  0.5f, -0.5f,  1.0f, 0.0f,   // C 13
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   // G 14
            0.5f, -0.5f,  0.5f,  0.0f, 1.0f,   // F 15

            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // A 16
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   // B 17
            0.5f, -0.5f,  0.5f,  1.0f, 1.0f,   // F 18
            -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  // E 19
            0.5f,  0.5f, -0.5f,   0.0f, 0.0f,  // C 20
            -0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  // D 21
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // H 22
            0.5f,  0.5f,  0.5f,   0.0f, 1.0f,  // G 23
    };
    uint32_t indexData[] = {
            0, 3, 2,
            2, 1, 0,
            4, 5, 6,
            6, 7 ,4,
            // left and right
            11, 8, 9,
            9, 10, 11,
            12, 13, 14,
            14, 15, 12,
            // bottom and top
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
    };

    auto buffer = MakeRef<Buffer>(context);
    buffer->create(sizeof(data),VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto destination = buffer->map();
    std::memcpy(destination,data,sizeof(data));
    buffer->unmap();

    auto indexBuffer = MakeRef<Buffer>(context);
    indexBuffer->create(sizeof(indexData),VK_BUFFER_USAGE_INDEX_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU);
     destination = indexBuffer->map();
    std::memcpy(destination,indexData,sizeof(indexData));
    indexBuffer->unmap();

    auto pipeline = MakeRef<Pipeline>(swapchain,shader,vertexInputState);
    pipeline->build();

    int width,height,lastWidth = 10, lastHeight = 10;
    glfwGetWindowSize(window, &width, &height);
    Nuc::Timer timer;
    double delta = 0;
    double gpuDelta = 0;
    while (!glfwWindowShouldClose(window)){
        timer.start();
        //glfwGetWindowSize(window, &width, &height);
        if(lastWidth != width || lastHeight != height){
            //resized
            lastWidth = width;
            lastHeight = height;
            vkDeviceWaitIdle(context->device);
            if(!swapchain->resize()){
                LOG_WARN("Failed to resize swapchain");
            }
            if(!pipeline->build()){
                LOG_WARN("Failed to rebuild pipeline");
            }
             perspective = Gem::PerspectiveInverseZ(90.f,(float)width,(float)height,0.01f);
        }
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            glfwSetWindowShouldClose(window,true);
        }
        if(gUpdate){
            bufferData = (ObjectLayout*)objectBuffer->map();
            for(int i = 0; i< objectCount;i++){
                bufferData[i].model = Gem::TranslationMatrix({random.next<float>(minRage,maxRange),random.next<float>(minRage,maxRange),random.next<float>(minRage,maxRange)});
                bufferData[i].color = {random.next<float>(0.0,1.0),random.next<float>(0.0,1.0),random.next<float>(0.0,1.0)};
            }
            objectBuffer->unmap();
        }

        camera.update();
        uint64_t timestamps[2] = {};
        if(vkGetQueryPoolResults(context->device,gpuTimeQueryPool,0,2,16,timestamps,sizeof(uint64_t),VK_QUERY_RESULT_64_BIT) == VK_SUCCESS){
            double begin = double(timestamps[0]) * properties.limits.timestampPeriod * 1e-6;
            double end = double(timestamps[1]) * properties.limits.timestampPeriod * 1e-6;
            gpuDelta = end - begin;
        }

        if(width != 0 && height != 0){
            swapchain->next({(float)std::sin(glfwGetTime()),0.4,(float)std::cos(glfwGetTime()),1.0});
            vkCmdResetQueryPool(context->commandBuffer,gpuTimeQueryPool,0,2);
            VkClearValue clearValues[2] ={
                    {1.0,1.0,1.0,1.0},
                    {0}
            };
            VkRenderPassBeginInfo renderPassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            renderPassBeginInfo.renderPass = swapchain->renderPass;
            renderPassBeginInfo.framebuffer = swapchain->framebuffers[swapchain->mImageIndex];
            renderPassBeginInfo.renderArea = {{0, 0}, {(uint32_t)width, (uint32_t)height}};
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValues;
            vkCmdBeginRenderPass(context->commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdWriteTimestamp(context->commandBuffer,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,gpuTimeQueryPool,0);
            vkCmdBindPipeline(context->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(context->commandBuffer,0,1,&buffer->buffer,&offset);
            vkCmdBindIndexBuffer(context->commandBuffer,indexBuffer->buffer,0,VK_INDEX_TYPE_UINT32);

           vkCmdBindDescriptorSets(context->commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline->layout,0,1,&context->descriptorSet,0, nullptr);
         //   pushDesc(context->commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline->layout,0,2,descriptorWrites);
            Gem::fMat4 model = perspective * camera.getViewMatrix()  ;
            vkCmdPushConstants(context->commandBuffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Gem::fMat4),model.data);
            vkCmdDrawIndexed(context->commandBuffer,sizeof(indexData) / sizeof(uint32_t),objectCount,0,0,0);
          //  vkCmdDrawIndexed(context->commandBuffer,sizeof(indexData) / sizeof(uint32_t),objectCount,0,0,0);


            vkCmdWriteTimestamp(context->commandBuffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,gpuTimeQueryPool,1);


            swapchain->flush();
        }
        glfwPollEvents();
        timer.stop();
        delta = timer.time<std::chrono::microseconds>()  / 1000.0;
        std::string title = fmt::format("CPU Time: {:.2f}ms  |  GPU Time: {:.2f}ms",delta,gpuDelta);
        glfwSetWindowTitle(window, title.c_str());
        timer.reset();
    }
    vkDeviceWaitIdle(context->device);
    vkDestroyQueryPool(context->device,gpuTimeQueryPool, nullptr);
    objectBuffer->destroy();
    sampler->destroy();
    image->destroy();
    buffer->destroy();
    indexBuffer->destroy();
    pipeline->terminate();
    shader->terminate();
    swapchain->terminate();
    vkDestroySurfaceKHR(context->instance,surface, nullptr);
    context->terminate();
    glfwTerminate();


    return 0;
}
