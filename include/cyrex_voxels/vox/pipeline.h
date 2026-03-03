//
// Created by Amelia on 28/02/2026.
//

#ifndef CYREX_VOXELS_PIPELINE_H
#define CYREX_VOXELS_PIPELINE_H

#include <cyrex_voxels/vox/voxel.h>
#include <array>

namespace vox {
	template<typename T>
	concept Selector = requires(T v)
	{
		{ v(Coord())} -> std::integral;
	};

	struct InvalidSelection{ int selection; };

	template <Selector Selector, VoxelSampler... Samplers>
	constexpr auto select(const Selector selector, const Samplers... samplers) {
		static_assert(sizeof...(Samplers) > 0);
		const auto tuple = std::tuple {samplers...};

		return [=](const Coord coord) {
			const auto selection = selector(coord);

			if (selection < 0 || selection >= static_cast<int>(sizeof...(Samplers))) {
				throw InvalidSelection{selection};
			}

			return [=]<std::size_t... I>(std::index_sequence<I...>) {
				using ReturnT = decltype(std::get<0>(tuple)(coord));
				ReturnT result{};
				bool found = false;
				(
					(selection == static_cast<int>(I)
						? (result = std::get<I>(tuple)(coord), found = true)
						: result),
					...
				);

				return result;
			}(std::make_index_sequence<sizeof...(Samplers)>{});
		};
	}
}

#endif //CYREX_VOXELS_PIPELINE_H