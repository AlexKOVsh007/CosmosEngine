#include "vk/Context.hpp"

#include "core/Window.hpp"

#include <GLFW/glfw3.h>

#include <array>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>

namespace {

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr std::array validationLayers{"VK_LAYER_KHRONOS_validation"};
constexpr std::array deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

bool validationLayersAvailable() {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());

    for (const char* wanted : validationLayers) {
        bool found = false;
        for (const VkLayerProperties& layer : available) {
            if (std::strcmp(wanted, layer.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT /*severity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*type*/,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* /*userData*/) {
    std::cerr << "[validation] " << data->pMessage << std::endl;
    return VK_FALSE;  // «сообщение принято, вызов Vulkan продолжай»
}

VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };
}

// Ищем семейство, умеющее и рисовать, и показывать в конкретный surface.
std::optional<uint32_t> findQueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

    for (uint32_t i = 0; i < count; ++i) {
        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
            continue;
        }
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport == VK_TRUE) {
            return i;
        }
    }
    return std::nullopt;
}

bool deviceExtensionsSupported(VkPhysicalDevice device) {
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());

    for (const char* required : deviceExtensions) {
        bool found = false;
        for (const VkExtensionProperties& ext : available) {
            if (std::strcmp(required, ext.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

// 0 — устройство непригодно; больше — лучше.
int rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface) {
    if (!findQueueFamily(device, surface).has_value()) {
        return 0;
    }
    if (!deviceExtensionsSupported(device)) {
        return 0;
    }

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(device, &props);

    switch (props.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return 3;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 2;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:            return 1;
        default:                                     return 0;
    }
}

std::vector<const char*> requiredInstanceExtensions() {
    uint32_t glfwCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwCount);
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

}  // namespace

Context::Context(const Window& window) {
    createInstance();
    createDebugMessenger();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
}

Context::~Context() {
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);

    if (debugMessenger != VK_NULL_HANDLE) {
        auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (destroy != nullptr) {
            destroy(instance, debugMessenger, nullptr);
        }
    }
    vkDestroyInstance(instance, nullptr);
}

void Context::createInstance() {
    // Проверяем заранее: иначе vkCreateInstance откажет безликим кодом.
    if (enableValidationLayers && !validationLayersAvailable()) {
        throw std::runtime_error("слои валидации запрошены, но не установлены");
    }

    const VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Cosmos",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "Cosmos Engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    const std::vector<const char*> extensions = requiredInstanceExtensions();

    // Временный messenger: покрывает валидацией сами vkCreateInstance/vkDestroyInstance.
    const VkDebugUtilsMessengerCreateInfoEXT debugInfo = debugMessengerInfo();

    const VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = enableValidationLayers ? &debugInfo : nullptr,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount =
            enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
        .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateInstance failed");
    }
}

void Context::createDebugMessenger() {
    if (!enableValidationLayers) {
        return;
    }

    // Функции расширений нет в таблице loader'а — адрес спрашиваем у instance.
    auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (create == nullptr) {
        throw std::runtime_error("vkCreateDebugUtilsMessengerEXT недоступна");
    }

    const VkDebugUtilsMessengerCreateInfoEXT info = debugMessengerInfo();
    if (create(instance, &info, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateDebugUtilsMessengerEXT failed");
    }
}

void Context::createSurface(const Window& window) {
    // GLFW сама выбирает нужное расширение платформы: X11, Wayland, Win32.
    if (glfwCreateWindowSurface(instance, window.getNativeHandle(), nullptr, &surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("glfwCreateWindowSurface failed");
    }
}

void Context::pickPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count == 0) {
        throw std::runtime_error("не найдено GPU с поддержкой Vulkan");
    }
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    int bestScore = 0;
    for (VkPhysicalDevice candidate : devices) {
        const int score = rateDevice(candidate, surface);
        if (score > bestScore) {
            bestScore = score;
            physicalDevice = candidate;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("ни одно GPU не подходит: нет очереди или swapchain");
    }

    queueFamily = findQueueFamily(physicalDevice, surface).value();

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    std::cout << "GPU: " << props.deviceName << ", семейство очередей " << queueFamily
              << std::endl;
}

void Context::createLogicalDevice() {
    // Приоритет обязателен: по числу на каждую заказанную очередь.
    const float queuePriority = 1.0f;
    const VkDeviceQueueCreateInfo queueInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    // Включаем только то, что железо действительно умеет и что нам нужно.
    VkPhysicalDeviceFeatures supported{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &supported);

    VkPhysicalDeviceFeatures enabledFeatures{};
    if (supported.samplerAnisotropy) {
        enabledFeatures.samplerAnisotropy = VK_TRUE;
    } else {
        std::cout << "samplerAnisotropy не поддерживается: текстуры будут мылить"
                  << std::endl;
    }

    const VkDeviceCreateInfo deviceInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledFeatures,
    };

    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateDevice failed");
    }

    vkGetDeviceQueue(device, queueFamily, 0, &graphicsQueue);
}
