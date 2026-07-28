#pragma once

#include <glm/vec3.hpp>

namespace deoxy::graphics {
    struct Vertex {
        glm::vec3 Position{0.0f};
        glm::vec3 Color{1.0f};
    };
}
