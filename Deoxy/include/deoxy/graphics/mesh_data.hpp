#pragma once

#include <deoxy/graphics/vertex.hpp>
#include <cstdint>
#include <vector>

namespace deoxy::graphics {
    struct MeshData {
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
    };
}
