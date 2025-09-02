#include "modules/ImGuiLayer.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <stdexcept>

void ImGuiLayer::init(const InitOptions& opts) {
    if (initialized) return;
    O = opts;

    // Create a small descriptor pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * (uint32_t)(sizeof(pool_sizes)/sizeof(pool_sizes[0]));
    pool_info.poolSizeCount = (uint32_t)(sizeof(pool_sizes)/sizeof(pool_sizes[0]));
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(O.device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
        throw std::runtime_error("ImGui: failed to create descriptor pool");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(O.window, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = O.instance;
    init_info.PhysicalDevice = O.physicalDevice;
    init_info.Device = O.device;
    init_info.QueueFamily = O.queueFamilyIndex;
    init_info.Queue = O.graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = O.imageCount;
    init_info.ImageCount = O.imageCount;
    init_info.MSAASamples = O.msaaSamples;
    init_info.CheckVkResultFn = nullptr;
    init_info.RenderPass = O.renderPass;
    ImGui_ImplVulkan_Init(&init_info);

    // Upload Fonts
    ImGui_ImplVulkan_CreateFontsTexture();
    ImGui_ImplVulkan_DestroyFontsTexture();

    initialized = true;
}

void ImGuiLayer::cleanup() {
    if (!initialized) return;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (imguiPool) {
        vkDestroyDescriptorPool(O.device, imguiPool, nullptr);
        imguiPool = VK_NULL_HANDLE;
    }
    initialized = false;
}

void ImGuiLayer::newFrame() {
    if (!initialized) return;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::renderDrawData(VkCommandBuffer cmd) {
    if (!initialized) return;
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

// no onSwapchainRecreated needed in this minimal hookup
