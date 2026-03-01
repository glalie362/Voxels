//
// Created by Amelia on 28/02/2026.
//

#ifndef CYREX_VOXELS_CONVERT_H
#define CYREX_VOXELS_CONVERT_H

#include <cyrex_voxels/vox/voxel.h>

namespace vox {
	template<Voxel From, Voxel To>
	struct voxel_convert_traits;

	template<Voxel From>
	struct voxel_convert_traits<From, From> {
		[[nodiscard]] constexpr From operator()(const From voxel) {
			return voxel;
		}
	};

	template<Voxel To>
	struct convert {
		template<VoxelSampler Sampler>
		[[nodiscard]] constexpr auto operator << (const Sampler& sampler) {
			using From = std::invoke_result_t<Sampler, Coord>;
			using ConvertTraits = voxel_convert_traits<From, To>;

			return [&](const Coord coord) {
				const auto from = sampler(coord);
				return ConvertTraits{}(from);
			};
		}
	};

}

#endif //CYREX_VOXELS_CONVERT_H