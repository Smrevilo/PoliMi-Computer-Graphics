// Minimal Dear ImGui layer for GLFW + Vulkan without depending on engine headers
#pragma once

#include <vulkan/vulkan.h>
struct GLFWwindow;
struct ImDrawData;

class ImGuiLayer {
public:
    struct InitOptions {
        GLFWwindow* window = nullptr;
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        uint32_t queueFamilyIndex = 0;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        uint32_t imageCount = 2;
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    };

    void init(const InitOptions& opts);
    void cleanup();
    void newFrame();
    void renderDrawData(VkCommandBuffer cmd);

private:
    InitOptions O{};
    VkDescriptorPool imguiPool = VK_NULL_HANDLE;
    bool initialized = false;
};
