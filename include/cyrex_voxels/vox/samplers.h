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
			return [=](const Coord& coord){
				return lhs(coord) && rhs(coord);
			};
		}

		[[nodiscard]] constexpr BooleanSampler auto operator || (const BooleanSampler auto& lhs, const BooleanSampler auto& rhs) {
			return [=](const Coord& coord){
				return lhs(coord) || rhs(coord);
			};
		}

		[[nodiscard]] constexpr BooleanSampler auto operator ^ (const BooleanSampler auto& lhs, const BooleanSampler auto& rhs) {
			return [=](const Coord& coord){
				return static_cast<bool>(lhs(coord) ^ rhs(coord));
			};
		}

		[[nodiscard]] constexpr BooleanSampler auto operator ! (const BooleanSampler auto& unary) {
			return [=](const Coord& coord){
				return !unary(coord);
			};
		}
	}

	namespace blend_ops {
		[[nodiscard]] constexpr VoxelSampler auto operator | (const VoxelSampler auto& lhs, const VoxelSampler auto& rhs) {
			return [=](const Coord& coord){
				return lhs(coord) | rhs(coord);
			};
		}
	}

	template<typename Op, VoxelSampler... Samplers>
	struct logical {
		std::tuple<Samplers...> samplers;

		constexpr logical() = default;
		constexpr logical(std::tuple<Samplers...> s) : samplers(s) {}

		template<VoxelSampler Sampler>
		[[nodiscard]] constexpr auto operator|(const Sampler sampler) const {
			return logical<Op,  Samplers..., Sampler>{
				std::tuple_cat(samplers, std::tuple{sampler})
			};
		}

		[[nodiscard]] constexpr bool operator()(const Coord& coord) const {
			if constexpr (sizeof...(Samplers) == 0) {
				return false; // or static_assert if you prefer
			} else {
				return std::apply([&](const auto& first, const auto&... rest) {
					Op op;
					bool result = first(coord);
					((result = op(result, rest(coord))), ...);
					return result;
				}, samplers);
			}
		}
	};

	[[nodiscard]] constexpr auto bool_to_voxel(const Voxel auto vox_true, const Voxel auto vox_false) {
		return [=](const bool cond, const Coord&) {
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

	[[nodiscard]] constexpr auto identity(const Voxel auto voxel) {
		return [=](const Coord&) {
			return voxel;
		};
	}

	template<VoxelToVoxel<bool> Converter>
	[[nodiscard]] constexpr auto bool_to_any(const BoolToVoxel auto fn,
		const Converter converter) {
		return [=](const Coord& coord) {
			return fn(coord) ? converter(true, coord) : converter(false, coord);
		};
	}

	template<BoolToVoxel Converter = decltype(bool_to_voxel(true, false))>
	[[nodiscard]] constexpr auto sphere(
		const Coord& origin,
		const float radius,
		Converter converter = bool_to_voxel(true, false)
	) {
		const float radius_squared = radius * radius;

		return [=](const Coord& coord) {
			const auto coord_float = glm::vec3(coord - origin);
			const auto dist_squared = glm::dot(coord_float, coord_float);
			return converter(dist_squared <= radius_squared, coord);
		};
	}

	template<BoolToVoxel Converter = decltype(bool_to_voxel(true, false))>
	[[nodiscard]] constexpr auto cylinder(
		const Coord& origin,
		const float radius,
		const int height,
		Converter converter = bool_to_voxel(true, false)
	) {
		const float radius_squared = radius * radius;
		const auto origin_xz = glm::vec2(origin.x,origin.z);

		return [=](const Coord& coord) {
			if (coord.y < origin.y) return false;
			if (coord.y > origin.y + height) return false;

			const auto coord_xz = glm::vec2(coord.x,coord.z);
			const auto coord_float = coord_xz - origin_xz;
			const auto dist_squared = glm::dot(coord_float, coord_float);
			return converter(dist_squared <= radius_squared, coord);
		};
	}

	template<BoolToVoxel Converter = decltype(bool_to_voxel(true, false))>
	[[nodiscard]] constexpr auto box(const Bounds bounds, const Converter converter = bool_to_voxel(true, false)) {
		return [=](const Coord& coord) {
			return converter(bounds.contains(coord), coord);
		};
	}
}

#endif //CYREX_VOXELS_SAMPLERS_H