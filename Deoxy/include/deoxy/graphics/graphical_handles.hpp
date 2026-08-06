#pragma once

#include <deoxy/core/resource_handle.hpp>

namespace deoxy::graphics {
    namespace detail {
        struct MeshHandleTag{};
        struct TextureHandleTag{};
        struct MaterialHandleTag{};
        struct PointLightTag{};
        struct SpotLightTag{};
    }

    using MeshHandle = core::ResourceHandle<detail::MeshHandleTag>;
    using TextureHandle = core::ResourceHandle<detail::TextureHandleTag>;
    using MaterialHandle = core::ResourceHandle<detail::MaterialHandleTag>;
    using PointLightHandle = core::ResourceHandle<detail::PointLightTag>;
    using SpotLightHandle = core::ResourceHandle<detail::SpotLightTag>;
}
