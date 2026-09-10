#pragma once

#include <string>

class Allocator;
class Commands;
class Context;
class Swapchain;

// Заставка на время загрузки сцены.
namespace splash {

// Логотип копируется прямо в картинку swapchain: ни pipeline, ни шейдеров не нужно.
void show(const Context& context, const Allocator& allocator, const Commands& commands,
          const Swapchain& swapchain, const std::string& path);

}  // namespace splash
