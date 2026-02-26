//
// Created by Amelia on 25/02/2026.
// Collection of helpful samplers

#ifndef CYREX_VOXELS_SAMPLERS_H
#define CYREX_VOXELS_SAMPLERS_H

#include <cyrex_voxels/vox/voxel.h>
#include <glm/geometric.hpp>
#include <concepts>

namespace vox {
	template<typename T>
	concept BoolToVoxel =
		std::invocable<T, bool, Coord> && Voxel<std::invoke_result<T, bool, Coord>>;

	template<typename Sampler>
	concept BooleanSampler =
		VoxelSampler<Sampler> &&
		std::same_as<bool, std::invoke_result_t<Sampler, Coord>>;

	namespace bool_ops {
		[[nodiscard]] constexpr BooleanSampler auto operator && (const BooleanSampler auto& lhs, const BooleanSampler auto& rhs) {
			return [=](const Coord coord){
				return lhs(coord) && rhs(coord);
			};
		}

		[[nodiscard]] constexpr BooleanSampler auto operator || (const BooleanSampler auto& lhs, const BooleanSampler auto& rhs) {
			return [=](const Coord coord){
				return lhs(coord) || rhs(coord);
			};
		}

		[[nodiscard]] constexpr BooleanSampler auto operator ^ (const BooleanSampler auto& lhs, const BooleanSampler auto& rhs) {
			return [=](const Coord coord){
				return lhs(coord) ^ rhs(coord);
			};
		}

		[[nodiscard]] constexpr BooleanSampler auto operator ! (const BooleanSampler auto& unary) {
			return [=](const Coord coord){
				return !unary(coord);
			};
		}
	}


	[[nodiscard]] constexpr auto make_bool_to_voxel(const Voxel auto vox_true, const Voxel auto vox_false) {
		return [=](const bool cond, const Coord) {
			return cond ? vox_true : vox_false;
		};
	}

	// Converts one generic voxel type to another generic voxel type
	template<typename T, typename V>
	concept VoxelToVoxel =
		Voxel<V> &&
		requires(T callable, V voxel, Coord coord)
	{
		{ callable(voxel, coord) } -> Voxel;
	};

	template<BoolToVoxel Converter = decltype(make_bool_to_voxel(true, false))>
	[[nodiscard]] constexpr auto make_sphere_sampler(
		const Coord origin,
		const float radius,
		Converter converter = make_bool_to_voxel(true, false)
	) {
		const float radius_squared = radius * radius;

		return [=](const Coord coord) {
			const auto coord_float = glm::vec3(coord - origin);
			const auto dist_squared = glm::dot(coord_float, coord_float);
			return converter(dist_squared <= radius_squared, coord);
		};
	}

}

#endif //CYREX_VOXELS_SAMPLERS_H