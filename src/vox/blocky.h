//
// Created by Jamie on 13/02/2026.
//

#ifndef VOXEL_GAME_BLOCKY_H
#define VOXEL_GAME_BLOCKY_H

#include "voxel.h"

namespace vox {
    template<VoxelSampler Sampler>
    [[nodiscard]] auto make_blocky_mesher(const Bounds& bounds) {
        std::vector<VoxelMesh> lookup_table;
        lookup_table.resize(0b1000000);

        // this mesh generator precomputes all the configurations of each voxel face
        for (size_t config = 0; config < 0b1000000; ++config) {
            const bool is_up_face    = config & 0b000001;
            const bool is_down_face  = config & 0b000010;
            const bool is_left_face  = config & 0b000100;
            const bool is_right_face = config & 0b001000;
            const bool is_front_face = config & 0b010000;
            const bool is_back_face  = config & 0b100000;

            auto& [vertices, indices] = lookup_table.at(config);

            enum { TopLeft, TopRight, BottomRight, BottomLeft, };

            const auto push_front_face = [&]() {
                const unsigned num_verts = static_cast<unsigned int>(vertices.size());
                indices.insert( indices.end(), {
                    num_verts + TopLeft, num_verts + TopRight, num_verts + BottomRight,
                    num_verts + BottomRight, num_verts + BottomLeft, num_verts + TopLeft
                });
            };

            const auto push_back_face = [&]() {
                const unsigned num_verts = static_cast<unsigned int>(vertices.size());

                indices.insert( indices.end(), {
                    num_verts + TopLeft, num_verts + BottomLeft, num_verts + BottomRight,
                    num_verts + BottomRight, num_verts + TopRight, num_verts + TopLeft
                });
            };

            constexpr glm::vec3 Up { 0.0f, 1.0f, 0.0f};
            constexpr glm::vec3 Down { 0.0f, -1.0f, 0.0f};
            constexpr glm::vec3 Left { -1.0f, 0.0f, 0.0f};
            constexpr glm::vec3 Right { 1.0f, 0.0f, 0.0f};
            constexpr glm::vec3 Front { 0.0f, 0.0f, -1.0f};
            constexpr glm::vec3 Back { 0.0f, 0.0f, 1.0f};

            using Vertex = VoxelMesh::Vertex;

            if (is_up_face) {
                push_front_face();
                vertices.insert(vertices.end(), {
                    Vertex{.position =  {0.0f, 1.0f, 1.0f}, .normal = Up},
                    Vertex{.position =  {1.0f, 1.0f, 1.0f}, .normal = Up},
                    Vertex{.position =  {1.0f, 1.0f, 0.0f}, .normal = Up},
                    Vertex{.position =  {0.0f,1.0f, 0.0f}, .normal = Up},
                });
            }

            if (is_down_face) {
                push_back_face();
                vertices.insert(vertices.end(), {
                    Vertex{.position =  {0.0f, 0.0f, 1.0f}, .normal = Down},
                    Vertex{.position =  {1.0f, 0.0f, 1.0f}, .normal = Down},
                    Vertex{.position =  {1.0f, 0.0f, 0.0f}, .normal = Down},
                    Vertex{.position =  {0.0f,0.0f, 0.0f}, .normal  = Down},
                });
            }

            if (is_left_face) {
                push_front_face();
                vertices.insert(vertices.end(), {
                    Vertex{.position =  {0.0f, 1.0f, 1.0f}, .normal = Left},
                    Vertex{.position =  {0.0f, 1.0f, 0.0f}, .normal = Left},
                    Vertex{.position =  {0.0f, 0.0f, 0.0f}, .normal = Left},
                    Vertex{.position =  {0.0f,0.0f, 1.0f}, .normal = Left},
                });
            }

            if (is_right_face) {
                push_back_face();
                vertices.insert(vertices.end(), {
                    Vertex{.position =  {1.0f, 1.0f, 1.0f}, .normal = Right},
                    Vertex{.position =  {1.0f, 1.0f, 0.0f}, .normal = Right},
                    Vertex{.position =  {1.0f, 0.0f, 0.0f}, .normal = Right},
                    Vertex{.position =  {1.0f,0.0f, 1.0f}, .normal = Right},
                });
            }

            if (is_front_face) {
                push_front_face();
                vertices.insert(vertices.end(), {
                    Vertex{.position =  {0.0f, 1.0f, 1.0f}, .normal = Front},
                    Vertex{.position =  {1.0f, 1.0f, 1.0f}, .normal = Front},
                    Vertex{.position =  {1.0f, 0.0f, 1.0f}, .normal = Front},
                    Vertex{.position =  {0.0f,0.0f, 1.0f}, .normal = Front}
                });
            }

            if (is_back_face) {
                push_front_face();
                vertices.insert(vertices.end(), {
                    Vertex{.position =  {0.0f, 1.0f, 0.0f}, .normal = Back},
                    Vertex{.position =  {1.0f, 1.0f, 0.0f}, .normal = Back},
                    Vertex{.position =  {1.0f, 0.0f, 0.0f}, .normal = Back},
                    Vertex{.position =  {0.0f,0.0f, 0.0f}, .normal = Back}
                });
            }
        }

        const auto mesher = [bounds, lookup_table](const Sampler& sampler) -> VoxelMesh {
            VoxelMesh mesh{};
            mesh.vertices.reserve(0xFFFF);
            mesh.indices.reserve(0xFFFF);

            constexpr glm::ivec3 Up {0, 1, 0};
            constexpr glm::ivec3 Down {0, -1, 0};
            constexpr glm::ivec3 Left {-1, 0, 0};
            constexpr glm::ivec3 Right {1, 0, 0};
            constexpr glm::ivec3 Front {0, 0, 1};
            constexpr glm::ivec3 Back {0, 0, -1};

            sample_each(bounds, sampler, [&](const glm::ivec3 p, const auto& voxel) {
                if (!voxel.is_solid()) return;

                const auto get = [&](const auto p) -> decltype(sampler(glm::ivec3())){
                    if (p.x < bounds.from.x || p.x >= bounds.to.x) return {};
                    if (p.y < bounds.from.y || p.y >= bounds.to.y) return {};
                    if (p.z < bounds.from.z || p.z >= bounds.to.z) return {};
                    return sampler(p);
                };

                // use this to look up the pre-computed shape of the voxel
                const std::uint8_t mask =
                        static_cast<std::uint8_t>(!get(p + Up).is_solid())
                    |   static_cast<std::uint8_t>(!get(p + Down).is_solid()) << 1
                    |   static_cast<std::uint8_t>(!get(p + Left).is_solid()) << 2
                    |   static_cast<std::uint8_t>(!get(p + Right).is_solid()) << 3
                    |   static_cast<std::uint8_t>(!get(p + Front).is_solid()) << 4
                    |   static_cast<std::uint8_t>(!get(p + Back).is_solid()) << 5;

                if (!mask) return;

                const auto num_vertices = mesh.vertices.size();
                const auto& lookup = lookup_table.at(mask);

                // Transform vertex (shift and apply coloring)
                for (const auto& vertex : lookup.vertices) {
                    auto transformed = vertex;
                    transformed.position += glm::vec3(p);
                    transformed.color = voxel.colorized();
                    mesh.vertices.emplace_back(transformed);
                }

                for (const auto indice  : lookup.indices) {
                    mesh.indices.emplace_back(indice + num_vertices);
                }
            });

            std::cerr << "num verts: " << mesh.vertices.size() << '\n';
            std::cerr << "num elems: " << mesh.indices.size() << '\n';
            return mesh;
        };


        return mesher;
    }

}

#endif //VOXEL_GAME_BLOCKY_H