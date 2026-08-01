#include <deoxy/graphics/mesh_generator.hpp>
#include <deoxy/graphics/vertex.hpp>
#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec3.hpp>

#include <cstdint>
#include <cassert>
#include <utility>
#include <vector>

namespace deoxy::graphics {
    // Como ainda não tem normais, não precisamos de tantos vertices
    MeshData MeshGenerator::CreateCube(float size) {
        assert(size > 0.0f);

        const float half = size * 0.5f;

        std::vector<Vertex> vertices {
            Vertex {
                .Position = math::Vec3{-half, -half, half},
                .Tint = Color{1.0f, 0.0f, 0.0f}
            },
            Vertex {
                .Position = math::Vec3{half, -half, half},
                .Tint = Color{0.0f, 1.0f, 0.0f}
            },
            Vertex {
                .Position = math::Vec3{half, half, half},
                .Tint = Color{0.0f, 0.0f, 1.0f}
            },
            Vertex {
                .Position = math::Vec3{-half, half, half},
                .Tint = Color{1.0f, 0.0f, 1.0f}
            },
            Vertex {
                .Position = math::Vec3{-half, -half, -half},
                .Tint = Color{1.0f, 1.0f, 0.0f}
            },
            Vertex {
                .Position = math::Vec3{half, -half, -half},
                .Tint = Color{0.0f, 1.0f, 1.0f}
            },
            Vertex {
                .Position = math::Vec3{half, half, -half},
                .Tint = Color{1.0f}
            },
            Vertex {
                .Position = math::Vec3{-half, half, -half},
                .Tint = Color{1.0f, 0.6f, 0.0f}
            },
        };

        std::vector<uint32_t> indices {
            // +Z
            0, 1, 2,
            0, 2, 3,

            // -Z
            5, 4, 7,
            5, 7, 6,

            // +X
            1, 5, 6,
            1, 6, 2,

            // -X
            4, 0, 3,
            4, 3, 7,

            // +Y
            3, 2, 6,
            3, 6, 7,

            // -Y
            4, 5, 1,
            4, 1, 0
        };

        return MeshData { std::move(vertices), std::move(indices) };
    }
}
