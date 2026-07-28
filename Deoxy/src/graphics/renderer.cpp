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

    bool Renderer::BeginFrame() {
        return m_impl->Context.BeginFrame();
    }

    void Renderer::EndFrame() {
        m_impl->Context.EndFrame();
    }

    void Renderer::SetClearColor(Color color) {
        m_impl->Context.SetClearColor(color);
    }

    void Renderer::SetClearColor(Color32 color32) {
        m_impl->Context.SetClearColor(ToColor(color32));
    }
}
