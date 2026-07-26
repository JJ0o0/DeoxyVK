#include <deoxy/graphics/renderer.hpp>

#include "vulkan/vk_context.hpp"

#include <memory>

namespace deoxy::graphics {
    struct Renderer::Impl {
        VulkanContext Context;

        explicit Impl(platform::Window& window) : Context(window) {}
    };

    Renderer::Renderer(platform::Window& window)
        : m_impl(std::make_unique<Impl>(window)) {}

    Renderer::~Renderer() = default;

    void Renderer::Render() {
        m_impl->Context.DrawFrame();
    }
}
