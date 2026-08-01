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

    MeshHandle Renderer::CreateMesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices) {
        return m_impl->Context.CreateMesh(vertices, indices);
    }

    MeshHandle Renderer::CreateMesh(const MeshData& data) {
        return m_impl->Context.CreateMesh(data.Vertices, data.Indices);
    }

    void Renderer::DestroyMesh(MeshHandle mesh) {
        m_impl->Context.DestroyMesh(mesh);
    }

    void Renderer::DrawMesh(MeshHandle mesh, const math::Mat4& modelMatrix) {
        m_impl->Context.DrawMesh(mesh, modelMatrix);
    }

    void Renderer::SetCamera(const math::Mat4& view, const math::Mat4& projection) {
        m_impl->Context.SetCamera(view, projection);
    }
}
