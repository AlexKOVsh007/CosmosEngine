#pragma once

#include "core/Camera.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "render/Mesh.hpp"
#include "render/Renderer.hpp"
#include "render/Texture.hpp"
#include "scene/Primitives.hpp"
#include "vk/Allocator.hpp"
#include "vk/Commands.hpp"
#include "vk/Context.hpp"
#include "vk/Descriptors.hpp"
#include "vk/Framebuffers.hpp"
#include "vk/Image.hpp"
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
    // Размер окна изменился: старые кадры и всё, что от них зависит, негодны.
    void recreateSwapchain();

    // Порядок объявления задаёт создание, а разрушение идёт снизу вверх.
    Window window;
    Camera camera;
    Input input;
    Context context;
    Allocator allocator;
    Swapchain swapchain;
    Image depthImage;
    RenderPass renderPass;
    Framebuffers framebuffers;
    Descriptors descriptors;
    Shader vertexShader;
    Shader fragmentShader;
    Pipeline pipeline;
    Commands commands;
    Mesh mesh;
    Texture texture;
    Renderer renderer;
};
