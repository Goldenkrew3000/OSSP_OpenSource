/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Debug / Prototype graphical interface
 */

/*
 * THIS IS SPECIFICALLY THE DEVELOPMENT INTERFACE
 * IT IS HORRIFICIALLY UNOPTIMIZED BUT IT WORKS
 */

#include "../external/imgui/imgui.h"
#include "../external/imgui/backends/imgui_impl_sdl2.h"
#include "../external/imgui/backends/imgui_impl_vulkan.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include "gui_entry.hpp"
#include "../configHandler.h"
#include "../libopensubsonic/httpclient.h"
#include "../libopensubsonic/endpoint_getStarred.h"
#include "../libopensubsonic/endpoint_getAlbum.h"
#include "../player/player.h"
#include "../libopensubsonic/endpoint_getInternetRadioStations.h"

#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

// Data
static VkAllocationCallbacks*   g_Allocator = nullptr;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static uint32_t                 g_MinImageCount = 2;
static bool                     g_SwapChainRebuild = false;

static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

static bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

static void SetupVulkan(ImVector<const char*> instance_extensions)
{
    VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkInitialize();
#endif

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // Enumerate available extensions
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
        check_vk_result(err);

        // Enable required extensions
        if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif

        // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;
        instance_extensions.push_back("VK_EXT_debug_report");
#endif

        // Create Vulkan Instance
        create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
        create_info.ppEnabledExtensionNames = instance_extensions.Data;
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
        volkLoadInstance(g_Instance);
#endif

        // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto f_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        err = f_vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
        check_vk_result(err);
#endif
    }

    // Select Physical Device (GPU)
    g_PhysicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(g_Instance);
    IM_ASSERT(g_PhysicalDevice != VK_NULL_HANDLE);

    // Select graphics queue family
    g_QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(g_PhysicalDevice);
    IM_ASSERT(g_QueueFamily != (uint32_t)-1);

    // Create Logical Device (with 1 queue)
    {
        ImVector<const char*> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // Enumerate physical device extension
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        const float queue_priority[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = g_QueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
        create_info.ppEnabledExtensionNames = device_extensions.Data;
        err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
        check_vk_result(err);
        vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
    }

    // Create Descriptor Pool
    // If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize& pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
}

static void CleanupVulkan()
{
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto f_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
    f_vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // APP_USE_VULKAN_DEBUG_REPORT

    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow()
{
    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkResult err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        g_SwapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(g_Device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    if (g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        g_SwapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}


extern configHandler_config_t* configObj;
bool bLikedSongsShow = false;
bool bAudioSettingsShow = false;
bool bPlayQueueShow = false;
bool bShowRadioStations = false;
bool bShowNowPlaying = false;
bool bShowLikedAlbums = false;
bool bShowLocalSongs = false;
void showLikedSongs();
void showAudioSettings();
void showPlayQueue();
void showRadioStations();
void showNowPlaying();
void showLikedAlbums();
void showLocalSongs();

int gui_entry() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("SDL could not be initialized: %s\n", SDL_GetError());
        return 1;
    }

    // Setup window
    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("OSSP v0.3a", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr) {
        printf("SDL could not create window: %s\n", SDL_GetError());
        return 1;
    }

ImVector<const char*> extensions;
    uint32_t extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, nullptr);
    extensions.resize(extensions_count);
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, extensions.Data);
    SetupVulkan(extensions);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err;
    if (SDL_Vulkan_CreateSurface(window, g_Instance, &surface) == 0)
    {
        printf("Failed to create Vulkan surface.\n");
        return 1;
    }

    // Create Framebuffers
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    // Scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // Setup platform/renderer backends
ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.RenderPass = wd->RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    // START START START
    ImVec4 background_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    bool done = false;

    while (!done) {
        // Poll events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done = true;
            }
        }
        
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Start new frame
ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("OSSP v0.3a");

            ImGui::Text("Connected to server at %s://%s", configObj->opensubsonic_protocol, configObj->opensubsonic_server);
            if (ImGui::Button("Liked Songs")) {
                printf("Liked songs button pressed\n");
                bLikedSongsShow = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Audio Settings")) {
                bAudioSettingsShow = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Play Queue")) {
                bPlayQueueShow = true;
            }

            if (ImGui::Button("Radio")) {
                bShowRadioStations = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Now Playing")) {
                bShowNowPlaying = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Liked Albums")) {
                bShowLikedAlbums = true;
            }

            if (ImGui::Button("Local Songs")) {
                bShowLocalSongs = true;
            }

            ImGui::End();
        }

        if (bLikedSongsShow) {
            showLikedSongs();
        }

        if (bAudioSettingsShow) {
            showAudioSettings();
        }

        if (bPlayQueueShow) {
            showPlayQueue();
        }

        if (bShowRadioStations) {
            showRadioStations();
        }

        if (bShowNowPlaying) {
            showNowPlaying();
        }

        if (bShowLikedAlbums) {
            showLikedAlbums();
        }

        if (bShowLocalSongs) {
            showLocalSongs();
        }

        // Render
        ImGui::Render();
ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            wd->ClearValue.color.float32[0] = background_color.x * background_color.w;
            wd->ClearValue.color.float32[1] = background_color.y * background_color.w;
            wd->ClearValue.color.float32[2] = background_color.z * background_color.w;
            wd->ClearValue.color.float32[3] = background_color.w;
            FrameRender(wd, draw_data);
            FramePresent(wd);
        }
    }

    // Cleanup
    //ImGui_ImplOpenGL2_Shutdown();
    //ImGui_ImplSDL2_Shutdown();
    //ImGui::DestroyContext();
    //SDL_GL_DeleteContext(gl_context);
    //SDL_DestroyWindow(window);
    //SDL_Quit();

    return 0;
}



bool haveLikedSongsInfo = false;
opensubsonic_getStarred_struct* starredStruct;
opensubsonic_httpClient_URL_t* starredUrl;

void getLikedSongsInfo() {
    // Pull liked songs
    starredUrl = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&starredUrl);
    starredUrl->endpoint = OPENSUBSONIC_ENDPOINT_GETSTARRED;
    opensubsonic_httpClient_formUrl(&starredUrl);
    opensubsonic_httpClient_fetchResponse(&starredUrl, (void**)&starredStruct);

    if (starredStruct->errorCode != 0) {
        // Error occured
    }

    haveLikedSongsInfo = true;
}

void showLikedSongs() {
    if (!haveLikedSongsInfo) { getLikedSongsInfo(); }

    ImGui::Begin("Liked Songs");

    ImGui::Text("Liked Songs");

    if (ImGui::Button("Close")) {
        bLikedSongsShow = false;
    }

    if (ImGui::Button("Refresh")) {
        opensubsonic_getStarred_struct_free(&starredStruct);
        opensubsonic_httpClient_URL_cleanup(&starredUrl);
        haveLikedSongsInfo = false;
    }

    static int selectedSong = -1;
    if (haveLikedSongsInfo) {
        if (ImGui::BeginChild("Liked Songs", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < starredStruct->songCount; i++) {
                if (ImGui::Selectable(starredStruct->songs[i].title, selectedSong == i)) {
                    selectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    if (selectedSong != -1) {
        // Form URL
        opensubsonic_httpClient_URL_t* song_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
        opensubsonic_httpClient_URL_prepare(&song_url);
        song_url->endpoint = OPENSUBSONIC_ENDPOINT_STREAM;
        song_url->id = strdup(starredStruct->songs[selectedSong].id);
        opensubsonic_httpClient_formUrl(&song_url);

        opensubsonic_httpClient_URL_t* coverart_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
        opensubsonic_httpClient_URL_prepare(&coverart_url);
        coverart_url->endpoint = OPENSUBSONIC_ENDPOINT_GETCOVERART;
        coverart_url->id = strdup(starredStruct->songs[selectedSong].coverArt);
        opensubsonic_httpClient_formUrl(&coverart_url);
        
        OSSPQ_AppendToEnd(starredStruct->songs[selectedSong].title,
                          starredStruct->songs[selectedSong].album,
                          starredStruct->songs[selectedSong].artist,
                          starredStruct->songs[selectedSong].id,
                          song_url->formedUrl,
                          coverart_url->formedUrl,
                          starredStruct->songs[selectedSong].duration,
                          OSSPQ_MODE_OPENSUBSONIC);
        
        opensubsonic_httpClient_URL_cleanup(&song_url);
        opensubsonic_httpClient_URL_cleanup(&coverart_url);
        selectedSong = -1;
    }

    ImGui::End();
}

float in_volume_val = 0;
float out_volume_val = 0;
float pitch_val = 0;
bool hasInVolumeFirstRun = false;
void showAudioSettings() {
    ImGui::Begin("Audio Settings");

    if (!hasInVolumeFirstRun) {
        in_volume_val = OSSPlayer_GstECont_InVolume_Get();
        out_volume_val = OSSPlayer_GstECont_OutVolume_Get();
        pitch_val = configObj->audio_pitch_cents / 100.0f; // Cents to semitones
        hasInVolumeFirstRun = true;
    }

    ImGui::Text("In Vol / Out Vol");

    // Idk what that field is, styling?, Size, Storage, Low, High
    if (ImGui::VSliderFloat("##invol", ImVec2(35, 160), &in_volume_val, 0.0f, 1.0f)) {
        // Data has changed
        OSSPlayer_GstECont_InVolume_set(in_volume_val);
    }

    ImGui::SameLine();

    if (ImGui::VSliderFloat("##outvol", ImVec2(35, 160), &out_volume_val, 0.0f, 1.0f)) {
        OSSPlayer_GstECont_OutVolume_set(out_volume_val);
    }

    ImGui::SameLine();

    if (ImGui::VSliderFloat("##pitch", ImVec2(35, 160), &pitch_val, -6.00f, 6.00f)) {
        OSSPlayer_GstECont_Pitch_Set(pitch_val * 100.0f); // Convert semitones to cents
    }

    if(ImGui::Button("Skip")) {
        OSSPlayer_GstECont_Playbin3_Stop();
    }

    if(ImGui::Button("Play/Pause")) {
        OSSPlayer_GstECont_Playbin3_PlayPause();
    }

    ImGui::End();
}

// TODO: go through abstraction
#include "../player/playQueue.hpp"

void showPlayQueue() {
    ImGui::Begin("Play Queue");

    static int selectedSong = -1;
            if (ImGui::BeginChild("Play Queue", ImVec2(0, 200), ImGuiChildFlags_Border)) {
                for (int i = 0; i < internal_OSSPQ_GetItemCount(); i++) {
                    if (ImGui::Selectable(internal_OSSPQ_GetTitleAtIndex(i), selectedSong == i)) {
                        selectedSong = i;
                    }
                }
            ImGui::EndChild();
            }

    ImGui::End();
}



bool haveRadioStations = false;
opensubsonic_getInternetRadioStations_struct* irsStruct;
opensubsonic_httpClient_URL_t* radioUrl;

void getRadioStations() {
    radioUrl = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&radioUrl);
    radioUrl->endpoint = OPENSUBSONIC_ENDPOINT_GETINTERNETRADIOSTATIONS;
    opensubsonic_httpClient_formUrl(&radioUrl);
    opensubsonic_httpClient_fetchResponse(&radioUrl, (void**)&irsStruct);

    if (irsStruct->errorCode != 0) {
        // Error happened
    }

    haveRadioStations = true;
}

void showRadioStations() {
    if (!haveRadioStations) { getRadioStations(); }

    ImGui::Begin("Radio Stations");

    static int selectedSong = -1;
    if (haveRadioStations) {
        if (ImGui::BeginChild("Radio Stations", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < irsStruct->radioStationCount; i++) {
                if (ImGui::Selectable(irsStruct->radioStations[i].name, selectedSong == i)) {
                    selectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    
    if (selectedSong != -1) {
        //OSSPlayer_QueueAppend_Radio(irsStruct->radioStations[selectedSong].name,
        //                      irsStruct->radioStations[selectedSong].id,
        //                      irsStruct->radioStations[selectedSong].streamUrl);
        selectedSong = -1;
    }

    ImGui::End();
}

void showNowPlaying() {
    ImGui::Begin("Now Playing");

    /*
     * Okay so I need:
     *  - Current and final position of song
     *  - Play, Pause, Next buttons (Needs DiscordRPC expansion)
     */

    ImGui::End();
}


void showAlbum(char* id);

int likedAlbumsSelectedSong = -1;
void showLikedAlbums() {
    // /getStarred is all of the liked info, not just songs
    if (!haveLikedSongsInfo) { getLikedSongsInfo(); }

    ImGui::Begin("Liked Albums");

    ImGui::Text("Liked Albums");

    if (ImGui::Button("Close")) {
        bShowLikedAlbums = false;
    }

    if (ImGui::Button("Refresh")) {
        opensubsonic_getStarred_struct_free(&starredStruct);
        opensubsonic_httpClient_URL_cleanup(&starredUrl);
        haveLikedSongsInfo = false;
    }

    if (haveLikedSongsInfo) {
        if (ImGui::BeginChild("Liked Albums", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < starredStruct->albumCount; i++) {
                if (ImGui::Selectable(starredStruct->albums[i].title, likedAlbumsSelectedSong == i)) {
                    likedAlbumsSelectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    if (likedAlbumsSelectedSong != -1) {
        showAlbum(starredStruct->albums[likedAlbumsSelectedSong].id);
    }

    ImGui::End();
}

bool hasAlbum = false;
opensubsonic_getAlbum_struct* getAlbumStruct;
void getAlbum(char* id) {
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETALBUM;
    url->id = strdup(id);
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getAlbumStruct);
    
    //opensubsonic_getAlbum_struct_free(&getAlbumStruct);
    //opensubsonic_httpClient_URL_cleanup(&url);

    hasAlbum = true;
}

static int rand_int(int n) {
  int limit = RAND_MAX - RAND_MAX % n;
  int rnd;

  do {
    rnd = rand();
  } while (rnd >= limit);
  return rnd % n;
}

void shuffle(int *array, int n) {
  int i, j, tmp;

  for (i = n - 1; i > 0; i--) {
    j = rand_int(i + 1);
    tmp = array[j];
    array[j] = array[i];
    array[i] = tmp;
  }
}

void shuffleAlbum() {
    int n = getAlbumStruct->songCount;

    int arr[1] = { 0 };
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }

    shuffle(arr, n);

    for (int i = 0; i < n; i++) {
        //OSSPlayer_QueueAppend_Song(getAlbumStruct->songs[arr[i]].title,
         //                     getAlbumStruct->songs[arr[i]].artist,
        //                      getAlbumStruct->songs[arr[i]].id,
        //                      getAlbumStruct->songs[arr[i]].duration);
    }
}

#include <ctime>
void showAlbum(char* id) {
    ImGui::Begin("Album");

    if (!hasAlbum) { getAlbum(id); }

    ImGui::Text("Album");

    if (ImGui::Button("Close")) {
        likedAlbumsSelectedSong = -1;
        hasAlbum = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Play all")) {
        for (int i = 0; i < getAlbumStruct->songCount; i++) {
            /*
            // Form URL
            opensubsonic_getAlbum_struct*

            opensubsonic_httpClient_URL_t* song_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
            opensubsonic_httpClient_URL_prepare(&song_url);
            song_url->endpoint = OPENSUBSONIC_ENDPOINT_STREAM;
            song_url->id = strdup(starredStruct->songs[selectedSong].id);
            opensubsonic_httpClient_formUrl(&song_url);

            opensubsonic_httpClient_URL_t* coverart_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
            opensubsonic_httpClient_URL_prepare(&coverart_url);
            coverart_url->endpoint = OPENSUBSONIC_ENDPOINT_GETCOVERART;
            coverart_url->id = strdup(starredStruct->songs[selectedSong].coverArt);
            opensubsonic_httpClient_formUrl(&coverart_url);

            OSSPQ_AppendToEnd(starredStruct->songs[selectedSong].title,
                              starredStruct->songs[selectedSong].album,
                              starredStruct->songs[selectedSong].artist,
                              starredStruct->songs[selectedSong].id,
                              song_url->formedUrl,
                              coverart_url->formedUrl,
                              starredStruct->songs[selectedSong].duration,
                              OSSPQ_MODE_OPENSUBSONIC);

            opensubsonic_httpClient_URL_cleanup(&song_url);
            opensubsonic_httpClient_URL_cleanup(&coverart_url);
            selectedSong = -1;                   getAlbumStruct->songs[i].duration);
            */
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Shuffle")) {
        srand(time(NULL));
        shuffleAlbum();
    }

    static int selectedSong = -1;
    if (hasAlbum) {
        if (ImGui::BeginChild("Album", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < getAlbumStruct->songCount; i++) {
                if (ImGui::Selectable(getAlbumStruct->songs[i].title, selectedSong == i)) {
                    selectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    ImGui::End();
}







#include "../localMusicHandler.hpp"

bool hasLocalSongs = false;
localMusicHandler_songReq_t* songReq;

void getLocalSongs() {
    songReq = localMusicHandler_test();
    hasLocalSongs = true;
}

void showLocalSongs() {
    

    ImGui::Begin("Local Songs");

    if (!hasLocalSongs) { getLocalSongs(); }

    ImGui::Text("Local Songs");

    static int selectedSong = -1;
    if (hasLocalSongs) {
        if (ImGui::BeginChild("LocalSongs", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < songReq->songCount; i++) {
                if (ImGui::Selectable(songReq->songs[i].title, selectedSong == i)) {
                    selectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    if (selectedSong != -1) {
        // Treat it as radio station for testing
        char* newPath = NULL;
        asprintf(&newPath, "file://%s", songReq->songs[selectedSong].path);

        //OSSPlayer_QueueAppend_Radio(songReq->songs[selectedSong].title,
        //                      songReq->songs[selectedSong].uid,
        //                      newPath);
        OSSPQ_AppendToEnd(songReq->songs[selectedSong].title,
                          NULL,
                          NULL,
                          songReq->songs[selectedSong].uid,
                          newPath,
                          NULL,
                          0,
                          OSSPQ_MODE_LOCALFILE);
        selectedSong = -1;
    }

    ImGui::End();
}
