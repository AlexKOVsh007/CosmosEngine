// Реализация VMA разворачивается ровно здесь и больше нигде.
#define VMA_IMPLEMENTATION
#include "vk/Allocator.hpp"

#include "vk/Context.hpp"

#include <stdexcept>

Allocator::Allocator(const Context& context) {
    const VmaAllocatorCreateInfo info{
        .physicalDevice = context.getPhysicalDevice(),
        .device = context.getDevice(),
        .instance = context.getInstance(),
        .vulkanApiVersion = VK_API_VERSION_1_0,
    };

    if (vmaCreateAllocator(&info, &allocator) != VK_SUCCESS) {
        throw std::runtime_error("vmaCreateAllocator failed");
    }
}

Allocator::~Allocator() {
    vmaDestroyAllocator(allocator);
}
