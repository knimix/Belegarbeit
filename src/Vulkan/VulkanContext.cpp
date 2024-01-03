#include "VulkanContext.h"
#include <vector>
#include <Nuc/Log.h>
#include <GLFW/glfw3.h>

VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT flags, const VkDebugUtilsMessengerCallbackDataEXT* data,  void* userdata){
    LOG_WARN("{}", data->pMessage);
    return VK_FALSE;
}

bool VulkanContext::initialize(bool validationLayer) {
    //creating instance
    LOG_INFO("Initializing Vulkan Context");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<VkValidationFeatureEnableEXT> enabledFeatures = {
          VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
         VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
    };
    VkValidationFeaturesEXT validationFeatures = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
    validationFeatures.enabledValidationFeatureCount = enabledFeatures.size();
   validationFeatures.pEnabledValidationFeatures = enabledFeatures.data();

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pNext = &validationFeatures;
    createInfo.pApplicationInfo = &appInfo;
    uint32_t extensionCount = 0;
    auto extensionData = glfwGetRequiredInstanceExtensions(&extensionCount);
    std::vector<const char*> extensions(extensionData, extensionData + extensionCount);
    if(validationLayer){
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions.emplace_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    }
    for (auto& extension : extensions) {
        LOG_DEBUG("Using extension: {}", extension);
    }
   std::vector<const char*> activeLayers;
    createInfo.ppEnabledLayerNames = nullptr;
    if(validationLayer){
       LOG_DEBUG("Using validation layers");
       activeLayers.emplace_back("VK_LAYER_KHRONOS_validation");
        createInfo.ppEnabledLayerNames = activeLayers.data();
    }
    createInfo.enabledLayerCount = activeLayers.size();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        LOG_WARN("Failed to create vulkan instance");
        return false;
    }

    //debugger
    if(validationLayer) {
        PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugMessenger;
        pfnCreateDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;
        pfnCreateDebugMessenger(instance, &debugCreateInfo, nullptr, &debugger);
    }
    //pick physical device

    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    LOG_DEBUG("Found {} device/s", deviceCount);
    physicalDevice = devices.at(0);

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice,&deviceProperties);
    LOG_DEBUG("Selected GPU: {}", deviceProperties.deviceName);

    //select queue family

    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilyProperties.data());
    mCommandQueueFamilyIndex = -1;
    for (int i = 0; i < queueFamiliesCount; i++) {
        VkQueueFamilyProperties properties = queueFamilyProperties[i];
        if (properties.queueCount > 0 && properties.queueFlags & VK_QUEUE_GRAPHICS_BIT && properties.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            mCommandQueueFamilyIndex = i;
            break;
        }
    }
    if (mCommandQueueFamilyIndex == -1) {
        LOG_WARN("Failed to find queue family with GRAPHICS_BIT and TRANSFER_BIT");
        return false;
    }


    //create device


    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = mCommandQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = priorities;

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = & queueCreateInfo;
    std::vector<const char*> deviceExtensions = {"VK_KHR_swapchain", VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        LOG_WARN("Failed to create device");
        return false;
    }
    LOG_DEBUG("Created device");
    //receive queue
    vkGetDeviceQueue(device, mCommandQueueFamilyIndex, 0, &commandQueue);


    //creating command pool and command buffers

    VkCommandPoolCreateInfo commandPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    commandPoolCreateInfo.queueFamilyIndex = mCommandQueueFamilyIndex;
    if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        LOG_WARN("Failed to create command pool");
        return false;
    }
    VkCommandBufferAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer) != VK_SUCCESS) {
        LOG_WARN("Failed to create command buffer");
        return false;
    }
    LOG_DEBUG("Created CommandPool and CommandBuffer");

    //create fence

    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS) {
        LOG_WARN("Failed to create fence");
        return false;
    }
    //crate semaphores

    VkSemaphoreCreateInfo acquireSemaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (vkCreateSemaphore(device, &acquireSemaphoreCreateInfo, nullptr, &acquireSemaphore) != VK_SUCCESS) {
        LOG_DEBUG("Failed to create acquire semaphore");
        return false;
    }
    VkSemaphoreCreateInfo releaseSemaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (vkCreateSemaphore(device, &releaseSemaphoreCreateInfo, nullptr, &submitSemaphore) != VK_SUCCESS) {
        LOG_DEBUG("Failed to create release semaphore");
        return false;
    }

    //creating vma allocator

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    //creating descriptor pool

    VkDescriptorPoolSize poolSize[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,2}
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.poolSizeCount = 2;
    descriptorPoolCreateInfo.pPoolSizes = poolSize;
    vkCreateDescriptorPool(device,&descriptorPoolCreateInfo, nullptr,&descriptorPool);

    //bindings

    VkDescriptorSetLayoutBinding bindings[] = {
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1,VK_SHADER_STAGE_VERTEX_BIT, nullptr}
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    descriptorSetCreateInfo.pBindings = bindings;
    descriptorSetCreateInfo.bindingCount = 2;
    vkCreateDescriptorSetLayout(device,&descriptorSetCreateInfo, nullptr,&descriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorAllocateInfo.descriptorPool = descriptorPool;
    descriptorAllocateInfo.descriptorSetCount = 1;
    descriptorAllocateInfo.pSetLayouts = &descriptorSetLayout;
   vkAllocateDescriptorSets(device,&descriptorAllocateInfo,&descriptorSet);

    return true;
}
void VulkanContext::terminate() const {
    LOG_DEBUG("Terminate application");
    vkDestroyDescriptorSetLayout(device,descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device,descriptorPool, nullptr);
    vmaDestroyAllocator(allocator);
    vkDestroySemaphore(device, acquireSemaphore, nullptr);
    vkDestroySemaphore(device, submitSemaphore, nullptr);
    vkDestroyFence(device, fence, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
  // PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugMessenger;
 //   pfnDestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  // pfnDestroyDebugMessenger(instance, debugger, nullptr);
    vkDestroyInstance(instance, nullptr);
}
