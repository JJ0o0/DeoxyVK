#include <deoxy/graphics/renderer.hpp>
#include <deoxy/graphics/image_loader.hpp>

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

    void Renderer::DrawMesh(MeshHandle mesh, MaterialHandle material, const math::Mat4& modelMatrix) {
        m_impl->Context.DrawMesh(mesh, material, modelMatrix);
    }

    TextureHandle Renderer::CreateTexture(const ImageData& data, const TextureCreateInfo& createInfo) {
        return m_impl->Context.CreateTexture(data, createInfo);
    }

    TextureHandle Renderer::CreateTexture(const std::filesystem::path& path, const TextureCreateInfo& createInfo) {
        ImageData data = ImageLoader::Load(path);
        return CreateTexture(data, createInfo);
    }

    void Renderer::DestroyTexture(TextureHandle handle) {
        m_impl->Context.DestroyTexture(handle);
    }

    MaterialHandle Renderer::CreateMaterial(const MaterialCreateInfo& createInfo) {
        return m_impl->Context.CreateMaterial(createInfo);
    }

    void Renderer::DestroyMaterial(MaterialHandle handle) {
        m_impl->Context.DestroyMaterial(handle);
    }

    std::optional<PointLightHandle> Renderer::CreatePointLight(const PointLight& light) {
        return m_impl->Context.CreatePointLight(light);
    }

    void Renderer::UpdatePointLight(PointLightHandle handle, const PointLight& light) {
        m_impl->Context.UpdatePointLight(handle, light);
    }

    void Renderer::DestroyPointLight(PointLightHandle handle) {
        m_impl->Context.DestroyPointLight(handle);
    }

    PointLight Renderer::GetPointLight(PointLightHandle handle) {
        return m_impl->Context.GetPointLight(handle);
    }

    void Renderer::SetCamera(const math::Mat4& view, const math::Mat4& projection) {
        m_impl->Context.SetCamera(view, projection);
    }

    void Renderer::SetDirectionalLight(const DirectionalLight& light) {
        m_impl->Context.SetDirectionalLight(light);
    }

    void Renderer::SetAmbientLight(const AmbientLight& light) {
        m_impl->Context.SetAmbientLight(light);
    }
}
