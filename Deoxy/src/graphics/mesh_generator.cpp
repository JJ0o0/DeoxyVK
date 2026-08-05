#include <deoxy/graphics/mesh_generator.hpp>
#include <deoxy/graphics/vertex.hpp>
#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec3.hpp>

#include <cstdint>
#include <cassert>
#include <utility>
#include <vector>

namespace deoxy::graphics {
    MeshData MeshGenerator::CreateCube(float size) {
        assert(size > 0.0f);

        const float half = size * 0.5f;

        std::vector<Vertex> vertices {
            // +Z
            Vertex{
                .Position = {-half, -half,  half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 0.0f},
                .Normal = {0.0f, 0.0f, 1.0f}
            },
            Vertex{
                .Position = { half, -half,  half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 0.0f},
                .Normal = {0.0f, 0.0f, 1.0f}
            },
            Vertex{
                .Position = { half,  half,  half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 1.0f},
                .Normal = {0.0f, 0.0f, 1.0f}
            },
            Vertex{
                .Position = {-half,  half,  half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 1.0f},
                .Normal = {0.0f, 0.0f, 1.0f}
            },

            // -Z
            Vertex{
                .Position = { half, -half, -half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 0.0f},
                .Normal = {0.0f, 0.0f, -1.0f}
            },
            Vertex{
                .Position = {-half, -half, -half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 0.0f},
                .Normal = {0.0f, 0.0f, -1.0f}
            },
            Vertex{
                .Position = {-half,  half, -half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 1.0f},
                .Normal = {0.0f, 0.0f, -1.0f}
            },
            Vertex{
                .Position = { half,  half, -half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 1.0f},
                .Normal = {0.0f, 0.0f, -1.0f}
            },

            // +X
            Vertex{
                .Position = {half, -half,  half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 0.0f},
                .Normal = {1.0f, 0.0f, 0.0f}
            },
            Vertex{
                .Position = {half, -half, -half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 0.0f},
                .Normal = {1.0f, 0.0f, 0.0f}
            },
            Vertex{
                .Position = {half,  half, -half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 1.0f},
                .Normal = {1.0f, 0.0f, 0.0f}
            },
            Vertex{
                .Position = {half,  half,  half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 1.0f},
                .Normal = {1.0f, 0.0f, 0.0f}
            },

            // -X
            Vertex{
                .Position = {-half, -half, -half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 0.0f},
                .Normal = {-1.0f, 0.0f, 0.0f}
            },
            Vertex{
                .Position = {-half, -half,  half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 0.0f},
                .Normal = {-1.0f, 0.0f, 0.0f}
            },
            Vertex{
                .Position = {-half,  half,  half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 1.0f},
                .Normal = {-1.0f, 0.0f, 0.0f}
            },
            Vertex{
                .Position = {-half,  half, -half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 1.0f},
                .Normal = {-1.0f, 0.0f, 0.0f}
            },

            // +Y
            Vertex{
                .Position = {-half, half,  half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 0.0f},
                .Normal = {0.0f, 1.0f, 0.0f}
            },
            Vertex{
                .Position = { half, half,  half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 0.0f},
                .Normal = {0.0f, 1.0f, 0.0f}
            },
            Vertex{
                .Position = { half, half, -half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 1.0f},
                .Normal = {0.0f, 1.0f, 0.0f}
            },
            Vertex{
                .Position = {-half, half, -half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 1.0f},
                .Normal = {0.0f, 1.0f, 0.0f}
            },

            // -Y
            Vertex{
                .Position = {-half, -half, -half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 0.0f},
                .Normal = {0.0f, -1.0f, 0.0f}
            },
            Vertex{
                .Position = { half, -half, -half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 0.0f},
                .Normal = {0.0f, -1.0f, 0.0f}
            },
            Vertex{
                .Position = { half, -half,  half},
                .Tint = Color{1.0f},
                .UV = {1.0f, 1.0f},
                .Normal = {0.0f, -1.0f, 0.0f}
            },
            Vertex{
                .Position = {-half, -half,  half},
                .Tint = Color{1.0f},
                .UV = {0.0f, 1.0f},
                .Normal = {0.0f, -1.0f, 0.0f}
            }
        };

        std::vector<uint32_t> indices {
            // +Z
            0, 1, 2,
            0, 2, 3,

            // -Z
            4, 5, 6,
            4, 6, 7,

            // +X
            8, 9, 10,
            8, 10, 11,

            // -X
            12, 13, 14,
            12, 14, 15,

            // +Y
            16, 17, 18,
            16, 18, 19,

            // -Y
            20, 21, 22,
            20, 22, 23
        };

        return MeshData { std::move(vertices), std::move(indices) };
    }
}
