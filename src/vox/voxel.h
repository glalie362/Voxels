//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_VOXEL_H
#define VOXEL_GAME_VOXEL_H

#include <concepts>
#include <glm/vec3.hpp>

namespace vox {
    template<typename V>
    concept Voxel = requires(V v) {
        { v.is_solid() } -> std::convertible_to<bool>;
        { v.colorized() } -> std::convertible_to<glm::vec3>;
        // TODO some sort of generic make_face feature that takes in vertex corner info
        // (as to not lock the user into a specific vertex structure)
    };

    // an invokable object that returns a voxel
    template<typename Sampler>
    concept VoxelSampler = requires(Sampler sampler, glm::ivec3 p) {
        requires Voxel<std::invoke_result_t<Sampler, glm::ivec3>>;
    };

    struct VoxelMesh {
        struct Vertex {
            // FIXME: we do not need vec3s
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 color;
        };
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    struct Bounds {
        glm::ivec3 from{};
        glm::ivec3 to{};
        // TODO: assert from < to

        [[nodiscard]] constexpr bool contains(const glm::ivec3 p) const noexcept {
            if (p.x < from.x || p.x > to.x) return false;
            if (p.y < from.y || p.y > to.y) return false;
            if (p.z < from.z || p.z > to.z) return false;
            return true;
        }
    };

    constexpr void each(const Bounds& bounds, auto fn) {
        for (int z = bounds.from.z; z < bounds.to.z; z++) {
            for (int y = bounds.from.y; y < bounds.to.y; y++) {
                for (int x = bounds.from.x; x < bounds.to.x; x++) {
                    fn(glm::ivec3(x,y,z));
                }
            }
        }
    }

    template<VoxelSampler Sampler>
    constexpr void sample_each(const Bounds& bounds, const Sampler& sampler, auto fn) {
        for (int z = bounds.from.z; z < bounds.to.z; z++) {
            for (int y = bounds.from.y; y < bounds.to.y; y++) {
                for (int x = bounds.from.x; x < bounds.to.x; x++) {
                    const glm::ivec3 p(x,y,z);
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