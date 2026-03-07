//
// Created by Amelia on 01/03/2026.
//

#ifndef CYREX_VOXELS_MARCHING_H
#define CYREX_VOXELS_MARCHING_H

#include <cyrex_voxels/vox/voxel.h>
#include <array>

#include <glm/ext/quaternion_geometric.hpp>

namespace vox {

    namespace marching_detail {
        extern const std::array<int, 256> edge_table;
        extern const int tri_table[256][16];

        constexpr Coord corner_offsets[8] = {
            {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0},
            {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}
        };

        constexpr int edge_to_corner[12][2] = {
            {0,1},{1,2},{2,3},{3,0},
            {4,5},{5,6},{6,7},{7,4},
            {0,4},{1,5},{2,6},{3,7}
        };
    }

    template<VoxelSampler Sampler>
    [[nodiscard]] constexpr auto make_marching_mesher(const Sampler& sampler) {
        return [=](const Bounds& bounds) -> VoxelMesh {
            using Voxel = std::invoke_result_t<Sampler, Coord>;
            using MeshTraits = voxel_mesh_traits<Voxel>;

            const auto get = [&](const Coord coord) -> Voxel{
                if (bounds.contains(coord)) return sampler(coord);
                return {};
            };

            VoxelMesh mesh{};
            mesh.vertices.reserve(0xFFFF);
            mesh.indices.reserve(0xFFFF);

            for (int z = bounds.from.z - 1; z <= bounds.to.z; ++z)
            for (int y = bounds.from.y - 1; y <= bounds.to.y; ++y)
            for (int x = bounds.from.x - 1; x <= bounds.to.x; ++x) {
                const    Coord base{x, y, z};
                std::uint8_t cube_index = 0;
                float values[8];
                glm::vec3 positions[8];
                Voxel neighbours[8];

                for (int i = 0; i < 8; ++i) {
                    const Coord p = base + marching_detail::corner_offsets[i];
                    const auto voxel = get(p);
                    neighbours[i] = voxel;

                    values[i] = MeshTraits::is_visible(voxel) ? 1.0f : 0.0f;
                    positions[i] = glm::vec3(p);

                    if (values[i] > 0.5f) {
                        cube_index |= (1 << i);
                    }
                }

                const int edges = marching_detail::edge_table[cube_index];
                if (edges == 0) continue;

                glm::vec3 vert_list[12];

                for (int i = 0; i < 12; ++i) {
                    if (!(edges & (1 << i))) continue;

                    const int a = marching_detail::edge_to_corner[i][0];
                    const int b = marching_detail::edge_to_corner[i][1];

                    // TODO: check if return type is of sampler is a float?
                    const float t = 0.5f; // midpoint (can improve with density interpolation)

                    const auto mix = [](const auto& a, const auto& b, const float t) {
                        return a * (1.0f - t) + b * t;
                    };

                    vert_list[i] = mix(positions[a], positions[b], t);
                }

                const auto& table = marching_detail::tri_table[cube_index];

                for (int i = 0; table[i] != -1; i += 3) {
                    const auto start_index = mesh.vertices.size();

                    for (int j = 0; j < 3; ++j) {
                        const int edge = table[i + j];

                        VoxelMesh::Vertex v{};
                        v.position = vert_list[edge];
                        v.normal = {0.0f, 1.0f, 0.0f};

                        const int a = marching_detail::edge_to_corner[edge][0];
                        const int b = marching_detail::edge_to_corner[edge][1];

                        const int corner = values[a] > 0.5f ? a : b;

                        const Coord coord = base + marching_detail::corner_offsets[corner];
                        const auto& neighbour_voxel = neighbours[corner];

                        v.color = MeshTraits::color(neighbour_voxel, coord);

                        mesh.vertices.emplace_back(v);
                    }

                    auto& v0 = mesh.vertices[start_index + 0];
                    auto& v1 = mesh.vertices[start_index + 1];
                    auto& v2 = mesh.vertices[start_index + 2];

                    const Normal normal = -glm::normalize(glm::cross(
                        v1.position - v0.position,
                        v2.position - v0.position
                    ));

                    v0.normal = normal;
                    v1.normal = normal;
                    v2.normal = normal;

                    mesh.indices.emplace_back(start_index + 2);
                    mesh.indices.emplace_back(start_index + 1);
                    mesh.indices.emplace_back(start_index + 0);
                }
            }

            return mesh;
        };
    }
}

#endif //CYREX_VOXELS_MARCHING_H