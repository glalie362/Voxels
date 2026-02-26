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

    template<typename Voxel>
    struct voxel_traits;


#pragma region Predefined traits

    // default traits:
    // white voxel if not zero
    template<typename Voxel>
    struct voxel_traits {
        [[nodiscard]] constexpr static bool is_solid(const Voxel& voxel) {
            if constexpr (std::equality_comparable_with<Voxel, int>) {
                return voxel > 0;
            }
            else {
                return false;
            }
        }

        [[nodiscard]] constexpr static Color color(const Voxel&, const Coord) {
            return Color(1.0f, 1.0f, 1.0f, 1.0f);
        }
    };

    template<typename T>
    concept ColoredVoxelWithIsSolidIndicator = requires(T t)
    {
        { t.is_solid } -> std::convertible_to<bool>;
        { t.color } -> std::convertible_to<Color>;
    };


    template<ColoredVoxelWithIsSolidIndicator V>
    struct voxel_traits<V> {
        [[nodiscard]] constexpr static bool is_solid(const V& voxel) {
            return voxel.is_solid;
        }

        [[nodiscard]] constexpr static Color color(const V& voxel, const Coord) {
            return voxel.color;
        }
    };

    template<>
    struct voxel_traits<bool> {
        [[nodiscard]] constexpr static bool is_solid(const bool voxel) {
            return voxel;
        }

        [[nodiscard]] constexpr static Color color(const bool, const Coord) {
            return Color(1.0f);
        }
    };

    template<>
    struct voxel_traits<Color> {
        [[nodiscard]] constexpr static bool is_solid(const Color voxel) {
            return voxel.a > 0.0f;
        }

        [[nodiscard]] constexpr static Color color(const Color voxel, const Coord) {
            return voxel;
        }
    };

#pragma endregion

    template<typename V>
    concept Voxel = requires(V v, Coord c) {
        { voxel_traits<V>::is_solid(v) } -> std::convertible_to<bool>;
        { voxel_traits<V>::color(v,c ) } -> std::convertible_to<Color>;
        // TODO some sort of generic make_face feature that takes in vertex corner info
        // (as to not lock the user into a specific vertex structure)
    };

    // an invokable object that returns a voxel
    template<typename Sampler>
    concept VoxelSampler = requires(Sampler sampler, Coord p) {
        requires Voxel<std::invoke_result_t<Sampler, Coord>>;
    };

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
    };

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

    template<typename Mesher, typename Sampler>
    concept VoxelMesher =
        VoxelSampler<Sampler> &&
        requires(Mesher mesher, const Sampler& sampler)
    {
        {mesher(sampler)} -> std::convertible_to<VoxelMesh>;
    };

}

#endif //VOXEL_GAME_VOXEL_H