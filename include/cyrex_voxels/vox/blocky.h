//
// Created by Amelia on 13/02/2026.
//

#ifndef VOXEL_GAME_BLOCKY_H
#define VOXEL_GAME_BLOCKY_H

#include <array>
#include <chrono>
#include <cyrex_voxels/vox/voxel.h>

namespace vox {
    namespace blocky_detail {
        extern const std::array<VoxelMesh, 64> lookup_table;
    }

    template<VoxelSampler Sampler>
    [[nodiscard]] auto make_blocky_mesher(const Sampler sampler) {
        return [sampler](const Bounds& bounds) -> VoxelMesh {
            VoxelMesh mesh{};
            mesh.vertices.reserve(0xFFFF);
            mesh.indices.reserve(0xFFFF);

            constexpr glm::ivec3 Up {0, 1, 0};
            constexpr glm::ivec3 Down {0, -1, 0};
            constexpr glm::ivec3 Left {-1, 0, 0};
            constexpr glm::ivec3 Right {1, 0, 0};
            constexpr glm::ivec3 Front {0, 0, 1};
            constexpr glm::ivec3 Back {0, 0, -1};

            sample_each(bounds, sampler, [&]<typename Voxel>(const Coord p, const Voxel voxel) {
                using traits = voxel_mesh_traits<Voxel>;
                if (!traits::is_visible(voxel)) return;

                const auto get = [&](const auto p) -> bool {
                    if (p.x < bounds.from.x || p.x > bounds.to.x) return false;
                    if (p.y < bounds.from.y || p.y > bounds.to.y) return false;
                    if (p.z < bounds.from.z || p.z > bounds.to.z) return false;
                    return traits::is_visible(sampler(p));
                };

                // use this to look up the pre-computed shape of the voxel
                const std::uint8_t mask =
                        (static_cast<std::uint8_t>(!get(p + Up))   << 0) |
                        (static_cast<std::uint8_t>(!get(p + Down)) << 1) |
                        (static_cast<std::uint8_t>(!get(p + Left)) << 2) |
                        (static_cast<std::uint8_t>(!get(p + Right)) << 3) |
                        (static_cast<std::uint8_t>(!get(p + Front)) << 4) |
                        (static_cast<std::uint8_t>(!get(p + Back)) << 5);

                if (!mask) return;


                const auto num_vertices = mesh.vertices.size();
                const auto& lookup = blocky_detail::lookup_table.at(mask);

                // Transform vertex (shift and apply coloring)
                for (const auto& vertex : lookup.vertices) {
                    auto transformed = vertex;
                    transformed.position += glm::vec3(p);
                    transformed.color = traits::color(voxel, p);
                    mesh.vertices.emplace_back(transformed);
                }

                for (const auto indice  : lookup.indices) {
                    mesh.indices.emplace_back(indice + num_vertices);
                }
            });

            return mesh;
        };
    }
}

#endif //VOXEL_GAME_BLOCKY_H