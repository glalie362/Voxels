//
// Created by Amelia on 01/03/2026.
//

#ifndef CYREX_VOXELS_CACHE_H
#define CYREX_VOXELS_CACHE_H

#include <cyrex_voxels/vox/voxel.h>

namespace vox {
	template <VoxelSampler Sampler>
	constexpr auto flat_cache(const Sampler& sampler, const Bounds bounds) {
		using Voxel = std::invoke_result_t<Sampler, Coord>;

		struct Cache {
			std::vector<Voxel> voxels;
			Bounds bounds;
			int width{};
			int height{};

			[[nodiscard]] constexpr Voxel operator ()(const Coord coord) const {
				if  (!bounds.contains(coord)) return Voxel{};
				// bounds may be negative
				const Coord local{
					coord.x - bounds.from.x,
					coord.y - bounds.from.y,
					coord.z - bounds.from.z
				};
				const int index = local.x + width * (local.y + height * local.z);
				return voxels[index];
			}

			explicit Cache(const Sampler& sampler, const Bounds& bounds) : bounds(bounds) {
				const auto size = bounds.size();
				width = size.x;
				height = size.y;
				const std::size_t volume = size.x * size.y * size.z;
				voxels.resize(volume);
				each(bounds, [&, index = 0](const Coord coord) mutable {
					voxels[index++] = sampler(coord);
				});
			}
		};

		return Cache(sampler, bounds);
	}

	// to do: octrees

}

#endif //CYREX_VOXELS_CACHE_H