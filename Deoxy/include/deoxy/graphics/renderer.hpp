#pragma once

#include <deoxy/graphics/lighting/directional_light.hpp>
#include <deoxy/graphics/lighting/ambient_light.hpp>
#include <deoxy/graphics/lighting/point_light.hpp>
#include <deoxy/graphics/lighting/spot_light.hpp>
#include <deoxy/graphics/graphical_handles.hpp>
#include <deoxy/graphics/mesh_data.hpp>
#include <deoxy/graphics/texture.hpp>
#include <deoxy/graphics/material.hpp>
#include <deoxy/graphics/vertex.hpp>
#include <deoxy/graphics/color.hpp>
#include <deoxy/math/math.hpp>

#include <filesystem>
#include <cstdint>
#include <memory>
#include <span>

namespace deoxy::platform {
    class Window;
}

namespace deoxy::graphics {
    class Renderer {
        public:
            Renderer(platform::Window& window);
            ~Renderer();

            bool BeginFrame();
            void EndFrame();

            void SetClearColor(Color color);
            void SetClearColor(Color32 color32);

            MeshHandle CreateMesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices);
            MeshHandle CreateMesh(const MeshData& data);
            void DestroyMesh(MeshHandle mesh);
            void DrawMesh(MeshHandle mesh, MaterialHandle material, const math::Mat4& modelMatrix);

            TextureHandle CreateTexture(const ImageData& data, const TextureCreateInfo& createInfo = {});
            TextureHandle CreateTexture(const std::filesystem::path& path, const TextureCreateInfo& createInfo = {});
            void DestroyTexture(TextureHandle handle);

            MaterialHandle CreateMaterial(const MaterialCreateInfo& createInfo);
            void DestroyMaterial(MaterialHandle handle);

            std::optional<PointLightHandle> CreatePointLight(const PointLight& light);
            void UpdatePointLight(PointLightHandle handle, const PointLight& light);
            void DestroyPointLight(PointLightHandle handle);
            PointLight GetPointLight(PointLightHandle handle);
            bool IsPointLightValid(PointLightHandle handle) const;

            std::optional<SpotLightHandle> CreateSpotLight(const SpotLight& light);
            void UpdateSpotLight(SpotLightHandle handle, const SpotLight& light);
            void DestroySpotLight(SpotLightHandle handle);
            SpotLight GetSpotLight(SpotLightHandle handle);
            bool IsSpotLightValid(SpotLightHandle handle) const;

            void SetCamera(const math::Mat4& view, const math::Mat4& projection);
            void SetDirectionalLight(const DirectionalLight& light);
            void SetAmbientLight(const AmbientLight& light);
        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;
    };
}
