#pragma once

#include <deoxy/core/resource_handle.hpp>

namespace deoxy::graphics {
    namespace detail {
        struct MeshHandleTag{};
        struct TextureHandleTag{};
    }

    using MeshHandle = core::ResourceHandle<detail::MeshHandleTag>;
    using TextureHandle = core::ResourceHandle<detail::TextureHandleTag>;
}
