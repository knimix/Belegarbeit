cmake_minimum_required(VERSION 3.25)
file(GLOB_RECURSE ENGINE_FILES src/Engine/*.cpp src/Engine/*.h src/Engine/*.c)
project(Belegarbeit)




project(VulkanRenderer)
file(GLOB_RECURSE SOURCE_FILES src/Vulkan/*.cpp src/Vulkan/*.h src/Vulkan/*.c)
find_package(glfw3)
find_package(Gem)
find_package(Nuc)
find_package(stb)
find_package(VulkanHeaders)
find_package(VulkanLoader)
find_package(vulkan-memory-allocator)
find_package(glm)
add_executable(VulkanRenderer src/Vulkan/main.cpp ${ENGINE_FILES} ${SOURCE_FILES})
target_link_libraries(VulkanRenderer glfw glm::glm Gem Nuc Vulkan::Loader vulkan-headers::vulkan-headers vulkan-memory-allocator::vulkan-memory-allocator stb::stb)

project(OpenGLRenderer)
file(GLOB_RECURSE SOURCE_FILES src/OpenGL/*.cpp src/OpenGL/*.h src/OpenGL/*.c)
find_package(glfw3)
find_package(Gem)
find_package(Nuc)
find_package(glad)
find_package(glm)


add_executable(OpenGLRenderer ${SOURCE_FILES} ${ENGINE_FILES})
target_link_libraries(OpenGLRenderer glm::glm glfw Gem Nuc glad::glad)