//
// Created by Amelia on 23/02/2026.
//

#include <cyrex_voxels/vox/blocky.h>

const std::array<vox::VoxelMesh, 64> vox::blocky_detail::lookup_table = []() {
    std::array<VoxelMesh, 64> lookup_table;

    // this mesh generator precomputes all the configurations of each voxel face
    for (size_t config = 0; config < lookup_table.size(); ++config) {
        const bool is_up_face    = config & 0b000001;
        const bool is_down_face  = config & 0b000010;
        const bool is_left_face  = config & 0b000100;
        const bool is_right_face = config & 0b001000;
        const bool is_front_face = config & 0b010000;
        const bool is_back_face  = config & 0b100000;

        auto& [vertices, indices] = lookup_table.at(config);

        enum { TopLeft, TopRight, BottomRight, BottomLeft, };
        const auto push_front_face = [&]() {
            const auto num_verts = static_cast<unsigned int>(vertices.size());

            indices.insert(indices.end(), {
               num_verts + TopLeft,     num_verts + TopRight,    num_verts + BottomRight,
               num_verts + BottomRight, num_verts + BottomLeft,  num_verts + TopLeft
           });
        };

        const auto push_back_face = [&]() {
            const auto num_verts = static_cast<unsigned int>(vertices.size());

            indices.insert(indices.end(), {
                num_verts + TopLeft,     num_verts + BottomLeft,  num_verts + BottomRight,
                num_verts + BottomRight, num_verts + TopRight,    num_verts + TopLeft
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
            push_back_face();
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
    return lookup_table;
}();