#pragma once

#include "core/Window.hpp"
#include "render/Renderer.hpp"
#include "vk/Commands.hpp"
#include "vk/Context.hpp"
#include "vk/Framebuffers.hpp"
#include "vk/Pipeline.hpp"
#include "vk/RenderPass.hpp"
#include "vk/Shader.hpp"
#include "vk/Swapchain.hpp"

// Владеет всем движком и запускает цикл отрисовки.
class CosmosEngine {
public:
    CosmosEngine();

    CosmosEngine(const CosmosEngine&) = delete;
    CosmosEngine& operator=(const CosmosEngine&) = delete;
    CosmosEngine(CosmosEngine&&) = delete;
    CosmosEngine& operator=(CosmosEngine&&) = delete;

    void run();

private:
    // Порядок объявления задаёт создание, а разрушение идёт снизу вверх.
    Window window;
    Context context;
    Swapchain swapchain;
    RenderPass renderPass;
    Framebuffers framebuffers;
    Shader vertexShader;
    Shader fragmentShader;
    Pipeline pipeline;
    Commands commands;
    Renderer renderer;
};
