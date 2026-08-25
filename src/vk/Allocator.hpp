#pragma once

#include <vk_mem_alloc.h>

class Context;

// Раздаёт видеопамять из больших блоков: аллокации драйвера дороги и лимитированы.
class Allocator {
public:
    explicit Allocator(const Context& context);
    ~Allocator();

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    VmaAllocator getHandle() const { return allocator; }

private:
    VmaAllocator allocator = VK_NULL_HANDLE;
};
