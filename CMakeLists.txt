# Cosmos Engine — рендерер на Vulkan.
cmake_minimum_required(VERSION 3.16)
project(CosmosEngine CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Путевой лист для редактора: чем и с какими флагами собирался каждый файл.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Слои валидации включаются только в Debug, поэтому он же по умолчанию.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# В Vulkan-структурах поля пропускаются сознательно: стандарт обнуляет их сам.
add_compile_options(-Wall -Wextra -Wno-missing-field-initializers)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Список ведём руками: file(GLOB) читает папку только на этапе cmake.
set(COSMOS_SOURCES
    src/main.cpp
    src/core/Window.cpp
    src/vk/Context.cpp
    src/vk/Swapchain.cpp
    src/vk/RenderPass.cpp
    src/vk/Framebuffers.cpp
    src/vk/Commands.cpp
    src/render/Frame.cpp
)

add_executable(cosmos ${COSMOS_SOURCES})

# Корень включений — src/. Из любого файла: #include "vk/Context.hpp".
target_include_directories(cosmos PRIVATE src)

target_link_libraries(cosmos PRIVATE Vulkan::Vulkan glfw)

# --- Шейдеры: GLSL → SPIR-V ------------------------------------------------
find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)

function(add_shader SRC OUT)
    add_custom_command(
        OUTPUT  ${CMAKE_SOURCE_DIR}/shaders/${OUT}
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/shaders/${SRC}
                                     -o ${CMAKE_SOURCE_DIR}/shaders/${OUT}
        DEPENDS ${CMAKE_SOURCE_DIR}/shaders/${SRC}
        COMMENT "Компиляция ${SRC} -> ${OUT}"
        VERBATIM)
endfunction()

add_shader(shader.vert vert.spv)
add_shader(shader.frag frag.spv)

# add_custom_command срабатывает, только если его результат кому-то нужен.
add_custom_target(shaders DEPENDS
    ${CMAKE_SOURCE_DIR}/shaders/vert.spv
    ${CMAKE_SOURCE_DIR}/shaders/frag.spv)

add_dependencies(cosmos shaders)
