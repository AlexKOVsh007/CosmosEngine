#pragma once

#include <vk_mem_alloc.h>

#include <cstddef>

class Allocator;
class Commands;

// Кусок памяти видеокарты; создаётся именованными фабриками, а не флагами.
class Buffer {
public:
    // Промежуточный: процессор пишет, видеокарта копирует к себе.
    [[nodiscard]] static Buffer staging(const Allocator& allocator, VkDeviceSize size);

    [[nodiscard]] static Buffer vertex(const Allocator& allocator, VkDeviceSize size);
    [[nodiscard]] static Buffer index(const Allocator& allocator, VkDeviceSize size);

    // Сразу с данными: заводят промежуточный буфер и копируют силами видеокарты.
    [[nodiscard]] static Buffer vertexFrom(const Allocator& allocator,
                                           const Commands& commands, const void* data,
                                           VkDeviceSize bytes);
    [[nodiscard]] static Buffer indexFrom(const Allocator& allocator,
                                          const Commands& commands, const void* data,
                                          VkDeviceSize bytes);

    // Обновляется каждый кадр, поэтому остаётся отображённым в память процессора.
    [[nodiscard]] static Buffer uniform(const Allocator& allocator, VkDeviceSize size);

    Buffer() = default;
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    VkBuffer getHandle() const { return buffer; }
    VkDeviceSize getSize() const { return size; }

    // Не nullptr только у буферов, созданных отображёнными.
    void* getMapped() const { return mapped; }

    // Копирует данные в отображённую память.
    void write(const void* data, size_t bytes) const;

    // Копирование делает сама видеокарта: быстрая память ей доступна, а нам нет.
    static void copy(const Commands& commands, const Buffer& source,
                     const Buffer& destination);

private:
    Buffer(const Allocator& allocator, VkDeviceSize size, VkBufferUsageFlags usage,
           VmaAllocationCreateFlags flags);

    void destroy() noexcept;

    // Доступ без владения.
    const Allocator* allocator = nullptr;

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    void* mapped = nullptr;
};
