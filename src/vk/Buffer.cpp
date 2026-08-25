#include "vk/Buffer.hpp"

#include "vk/Allocator.hpp"
#include "vk/Commands.hpp"

#include <cstring>
#include <stdexcept>
#include <utility>

Buffer::Buffer(const Allocator& allocatorRef, VkDeviceSize bufferSize,
               VkBufferUsageFlags usage, VmaAllocationCreateFlags flags)
    : allocator(&allocatorRef), size(bufferSize) {
    const VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    // AUTO: память подбирается по назначению буфера, а не перебором битов.
    const VmaAllocationCreateInfo allocInfo{
        .flags = flags,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VmaAllocationInfo result{};
    if (vmaCreateBuffer(allocator->getHandle(), &bufferInfo, &allocInfo, &buffer,
                        &allocation, &result) != VK_SUCCESS) {
        throw std::runtime_error("vmaCreateBuffer failed");
    }

    mapped = result.pMappedData;
}

Buffer Buffer::staging(const Allocator& allocator, VkDeviceSize size) {
    return Buffer(allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                      VMA_ALLOCATION_CREATE_MAPPED_BIT);
}

Buffer Buffer::vertex(const Allocator& allocator, VkDeviceSize size) {
    return Buffer(allocator, size,
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);
}

Buffer Buffer::index(const Allocator& allocator, VkDeviceSize size) {
    return Buffer(allocator, size,
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);
}

namespace {

// Быстрая память процессору недоступна: копирует туда сама видеокарта.
Buffer uploadThroughStaging(const Allocator& allocator, const Commands& commands,
                            const void* data, VkDeviceSize bytes, Buffer target) {
    Buffer staging = Buffer::staging(allocator, bytes);
    staging.write(data, static_cast<size_t>(bytes));
    Buffer::copy(commands, staging, target);
    return target;
}

}  // namespace

Buffer Buffer::vertexFrom(const Allocator& allocator, const Commands& commands,
                          const void* data, VkDeviceSize bytes) {
    return uploadThroughStaging(allocator, commands, data, bytes,
                                Buffer::vertex(allocator, bytes));
}

Buffer Buffer::indexFrom(const Allocator& allocator, const Commands& commands,
                         const void* data, VkDeviceSize bytes) {
    return uploadThroughStaging(allocator, commands, data, bytes,
                                Buffer::index(allocator, bytes));
}

Buffer Buffer::uniform(const Allocator& allocator, VkDeviceSize size) {
    return Buffer(allocator, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                      VMA_ALLOCATION_CREATE_MAPPED_BIT);
}

Buffer::~Buffer() {
    destroy();
}

void Buffer::destroy() noexcept {
    if (buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator->getHandle(), buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        mapped = nullptr;
        size = 0;
    }
}

void Buffer::write(const void* data, size_t bytes) const {
    if (mapped == nullptr) {
        throw std::runtime_error("буфер не отображён в память процессора");
    }
    std::memcpy(mapped, data, bytes);
}

void Buffer::copy(const Commands& commands, const Buffer& source,
                  const Buffer& destination) {
    VkCommandBuffer commandBuffer = commands.beginSingleTime();

    const VkBufferCopy region{.size = source.getSize()};
    vkCmdCopyBuffer(commandBuffer, source.getHandle(), destination.getHandle(), 1,
                    &region);

    commands.endSingleTime(commandBuffer);
}

Buffer::Buffer(Buffer&& other) noexcept
    : allocator(other.allocator),
      buffer(std::exchange(other.buffer, VK_NULL_HANDLE)),
      allocation(std::exchange(other.allocation, VK_NULL_HANDLE)),
      size(std::exchange(other.size, 0)),
      mapped(std::exchange(other.mapped, nullptr)) {}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        destroy();
        allocator = other.allocator;
        buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
        allocation = std::exchange(other.allocation, VK_NULL_HANDLE);
        size = std::exchange(other.size, 0);
        mapped = std::exchange(other.mapped, nullptr);
    }
    return *this;
}
