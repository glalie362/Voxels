//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_VOXEL_H
#define VOXEL_GAME_VOXEL_H

#include <concepts>
#include <span>

namespace vox {

    template<typename V>
    concept Voxel = requires(V v)
    {
        { v.is_solid() } -> std::convertible_to<bool>;
    };

    struct Vertex {

    };

    template<typename VM>
    concept VoxelMesher = requires(VM&& vm)
    {
        {vm()} -> std::convertible_to<std::span<const Vertex>>;
    };

    template<Voxel V>
    class Chunk {

    };
}

#endif //VOXEL_GAME_VOXEL_H