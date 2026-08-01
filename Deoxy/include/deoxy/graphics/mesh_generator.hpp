#pragma once

#include <deoxy/graphics/mesh_data.hpp>

namespace deoxy::graphics {
    class MeshGenerator {
        public:
            static MeshData CreateCube(float size = 1.0f);
    };
}
