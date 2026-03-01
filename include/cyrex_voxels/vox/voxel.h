//
// Created by Amelia on 11/02/2026.
//

#ifndef VOXEL_GAME_VOXEL_H
#define VOXEL_GAME_VOXEL_H

#include <concepts>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace vox {
    using Color = glm::vec4;
    using Normal = glm::vec3;
    using Coord = glm::ivec3;

    // A template specialization of this is required!
    template<typename Voxel>
    struct voxel_mesh_traits;

    template<typename V>
    concept Voxel = requires(V v, Coord c) {
        { voxel_mesh_traits<V>::is_visible(v) } -> std::same_as<bool>;
        { voxel_mesh_traits<V>::color(v,c ) } -> std::convertible_to<Color>;
        // TODO some sort of generic make_face feature that takes in vertex corner info
        // (as to not lock the user into a specific vertex structure)
    };

    // An invokable object that returns a voxel
    // Usage: sampler(Coord) -> Voxel
    template<typename Sampler>
    concept VoxelSampler = Voxel<std::invoke_result_t<Sampler, Coord>>;

    struct VoxelMesh {
        struct Vertex {
            // FIXME: we do not need vec3s
            glm::vec3 position;
            Normal normal;
            Color color;
        };
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    template<typename Mesher, typename Sampler>
    concept VoxelMesher =
     VoxelSampler<Sampler> &&
     requires(Mesher mesher, const Sampler& sampler)
    {
        {mesher(sampler)} -> std::convertible_to<VoxelMesh>;
    };

#pragma region Predefined traits

    // default traits:
    // white voxel if not zero
    template<typename Voxel>
    struct voxel_mesh_traits {
        [[nodiscard]] constexpr static bool is_visible(const Voxel& voxel) {
            if constexpr (std::equality_comparable_with<Voxel, int>) {
                return voxel > 0;
            }
            return false;
        }

        [[nodiscard]] constexpr static Color color(const Voxel&, const Coord) {
            return Color(1.0f);
        }
    };

    template<>
    struct voxel_mesh_traits<bool> {
        [[nodiscard]] constexpr static bool is_visible(const bool voxel) {
            return voxel;
        }

        [[nodiscard]] constexpr static Color color(const bool, const Coord) {
            return Color(1.0f);
        }
    };

    template<>
    struct voxel_mesh_traits<Color> {
        [[nodiscard]] constexpr static bool is_visible(const Color voxel) {
            return voxel.a > 0.0f;
        }

        [[nodiscard]] constexpr static Color color(const Color voxel, const Coord) {
            return voxel;
        }
    };

#pragma endregion

    struct Bounds {
        Coord from{};
        Coord to{};
        // TODO: assert from < to

        [[nodiscard]] constexpr bool contains(const Coord p) const noexcept {
            if (p.x < from.x || p.x > to.x) return false;
            if (p.y < from.y || p.y > to.y) return false;
            if (p.z < from.z || p.z > to.z) return false;
            return true;
        }

        [[nodiscard]] constexpr Coord size() const noexcept {
            return {
                to.x - from.x + 1,
                to.y - from.y + 1,
                to.z - from.z + 1
            };
        }
    };

    [[nodiscard]] constexpr Bounds cube_bounds(const int size) {
        const int half_size = size / 2;

        return {
            .from = {-half_size, -half_size, -half_size},
            .to = {half_size, half_size, half_size},
        };
    }

    constexpr void each(const Bounds& bounds, auto fn) {
        for (int z = bounds.from.z; z <= bounds.to.z; z++) {
            for (int y = bounds.from.y; y <= bounds.to.y; y++) {
                for (int x = bounds.from.x; x <= bounds.to.x; x++) {
                    fn(Coord(x,y,z));
                }
            }
        }
    }

    template<VoxelSampler Sampler>
    constexpr void sample_each(const Bounds& bounds, const Sampler& sampler, auto fn) {
        for (int z = bounds.from.z; z <= bounds.to.z; z++) {
            for (int y = bounds.from.y; y <= bounds.to.y; y++) {
                for (int x = bounds.from.x; x <= bounds.to.x; x++) {
                    const Coord p(x,y,z);
                    fn(p, sampler(p));
                }
            }
        }
    }

}

#endif //VOXEL_GAME_VOXEL_H