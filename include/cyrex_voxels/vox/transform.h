//
// Created by Amelia on 26/02/2026.
//

#ifndef CYREX_VOXELS_TRANSFORM_H
#define CYREX_VOXELS_TRANSFORM_H

#include <cyrex_voxels/vox/samplers.h>
#include "glm/gtc/noise.hpp"

namespace vox {
	template<typename T>
	concept Transformer = requires(T v)
	{
		{ v(Coord())} -> std::same_as<Coord>;
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

		template<Transformer Op>
		constexpr auto operator << (const Op op) const {
			return transformer<Ops..., Op>{
				std::tuple_cat(ops, std::tuple{op})
			};
		}

		template<VoxelSampler S>
		requires (!Transformer<S>)
		constexpr auto operator<<(S sampler) const {
			auto ops_copy = ops;
			return [ops_copy, sampler](const Coord coord) {
				Coord p = coord;
				std::apply([&](const auto&... op) {
					((p = op(p)), ...);
				}, ops_copy);
				return sampler(p);
			};
		}
	};

	struct transform {
		template<Transformer Op>
		[[nodiscard]] constexpr auto operator<<(const Op op) const {
			return transformer<Op>{ std::tuple{op} };
		}

		template<VoxelSampler Sampler>
		requires (!Transformer<Sampler>)	// prevent compiler from self confusion
		[[nodiscard]] constexpr auto operator<<(const Sampler sampler) const {
			return sampler;
		}
	};

	[[nodiscard]] constexpr auto translate(const Coord translation) {
		return transform_op {
		[=](const Coord coord) {
				return coord + translation;
			}
		};
	}

	[[nodiscard]] constexpr auto scale(const glm::vec3 scale) {
		const auto inv_scale = 1.0f / scale;
		return transform_op {
		[=](const Coord coord) {
				return Coord(glm::vec3(coord) * inv_scale);
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

	[[nodiscard]] constexpr auto repeat(const Coord period) {
		return transform_op {
		[=](const Coord coord) {
				return Coord(
					glm::mod(glm::vec3(coord), glm::vec3(period))
				);
			}
		};
	}

	[[nodiscard]] constexpr auto snap(const int size) {
		return transform_op {
		[=](const Coord coord) {
				return Coord(
					(coord / size) * size
				);
			}
		};
	}

	enum class Axis {
		X, Y, Z
	};

	template<Axis Axis>
	[[nodiscard]] constexpr auto mirror() {
		return transform_op{
			[=](const Coord coord) {
				if constexpr (Axis == Axis::X) return Coord(std::abs(coord.x), coord.y, coord.z);
				if constexpr (Axis == Axis::Y) return Coord(coord.x, std::abs(coord.y), coord.z);
				if constexpr (Axis == Axis::Z) return Coord(coord.x, coord.y, std::abs(coord.z));
			}
		};
	}

	template<Axis Axis>
	[[nodiscard]] constexpr auto twist(const float angle_degrees) {
		const float angle_radians = glm::radians(angle_degrees);

		const glm::vec3 vec_axis = [] {
			if constexpr(Axis == Axis::X) return glm::vec3(1, 0, 0);
			if constexpr(Axis == Axis::Y) return glm::vec3(0, 1, 0);
			if constexpr(Axis == Axis::Z) return glm::vec3(0, 0, 1);
		}();

		return transform_op {
			[=](const Coord coord) {
				const float scalar = [coord] {
					if constexpr(Axis == Axis::X) return coord.x;
					if constexpr(Axis == Axis::Y) return coord.y;
					if constexpr(Axis == Axis::Z) return coord.z;
				}();
				const float angle = scalar * angle_radians;
				const glm::quat q = glm::angleAxis(angle, vec_axis);
				return Coord(q * glm::vec3(coord));
			}
		};
	}

	template<typename T>
	concept NoiseGenerator = requires(T t, const glm::vec3 p)
	{
		{ t(p) } -> std::convertible_to<float>;
	};

	template <NoiseGenerator NoiseGen>
	[[nodiscard]] constexpr auto warp(const float scale, NoiseGen noise_gen) {
		constexpr auto epsilon = 1e-4;
		constexpr auto epsilon_x = glm::vec3(epsilon, 0.0f, 0.0f);
		constexpr auto epsilon_y = glm::vec3(0.0f, epsilon, 0.0f);
		constexpr auto epsilon_z = glm::vec3(0.0f, 0.0f, epsilon);

		return transform_op {
			[=](const Coord coord) {
				const auto coordf = glm::vec3(coord);
				const auto n  = noise_gen(coordf);
				const auto nx = noise_gen(coordf + epsilon_x);
				const auto ny = noise_gen(coordf + epsilon_y);
				const auto nz = noise_gen(coordf + epsilon_z);
				return Coord(coordf + (scale * glm::vec3(n - nx, n - ny, n - nz)));
			}
		};
	}

	[[nodiscard]] constexpr auto warp(const float scale) {
		const auto simplex_noise = [](const glm::vec3 p) {
			return glm::simplex(p);
		};
		return warp(scale, simplex_noise);
	}
}

#endif //CYREX_VOXELS_TRANSFORM_H