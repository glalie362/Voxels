//
// Created by Amelia on 26/02/2026.
//

#ifndef CYREX_VOXELS_TRANSFORM_H
#define CYREX_VOXELS_TRANSFORM_H

#include <cyrex_voxels/vox/samplers.h>
#include <functional>

#include "transform.h"
#include "transform.h"

namespace vox {
	template<typename T>
	concept Transformer = requires(T v)
	{
		{ v(Coord())} -> std::convertible_to<Coord>;
	};


	template<Transformer T>
	struct transform_op {
		T op;

		constexpr auto operator()(const Coord coord) const {
			return op(coord);
		}
	};


	template<typename... Ops>
	struct transformer {
		std::tuple<Ops...> ops;

		constexpr transformer() = default;
		constexpr transformer(const std::tuple<Ops...> o) : ops(o) {}

		template<typename Op>
		constexpr auto operator << (const Op op) const {
			return transformer<Ops..., Op>{
				std::tuple_cat(ops, std::tuple{op})
			};
		}

		template<VoxelSampler S>
		constexpr auto operator<<(S sampler) const {
			return [=](Coord c) {
				Coord p = c;
				std::apply([&](auto const&... op) {
					((p = op(p)), ...);
				}, ops);
				return sampler(p);
			};
		}
	};

	struct transform {
		template<typename Op>
		[[nodiscard]] constexpr auto operator<<(const Op op) const {
			return transformer<Op>{ std::tuple{op} };
		}
	};

	[[nodiscard]] constexpr auto translate(const Coord translation) {
		return transform_op {
			[=](const Coord coord) {
				return coord + translation;
			}
		};
	}

	[[nodiscard]] constexpr auto rotate(const glm::quat rotation) {
		return transform_op {
			[=](const Coord coord) {
				return Coord(rotation * glm::vec3(coord));
			}
		};
	}

	enum class MirrorAxis { X, Y, Z };

	template<MirrorAxis Axis>
	[[nodiscard]] constexpr auto mirror(const int mirror_line = 0) {
		return transform_op{
			[=](const Coord coord) {
				if constexpr (Axis == MirrorAxis::X)
					return Coord(2 * mirror_line - coord.x, coord.y, coord.z);
				else if constexpr (Axis == MirrorAxis::Y)
					return Coord(coord.x, 2 * mirror_line - coord.y, coord.z);
				else
					return Coord(coord.x, coord.y, 2 * mirror_line - coord.z);
			}
		};
	}

}

#endif //CYREX_VOXELS_TRANSFORM_H