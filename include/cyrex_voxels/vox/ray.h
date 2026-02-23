//
// Created by Amelia on 15/02/2026.
//

#ifndef CYREX_VOXELS_RAY_H
#define CYREX_VOXELS_RAY_H
#include <cyrex_voxels/vox/voxel.h>
#include <glm/glm.hpp>
#include <optional>

namespace vox {

	template<VoxelSampler Sampler>
	auto make_raycaster(const Sampler sampler, const Bounds bounds) {
		using Voxel = decltype(sampler(glm::ivec3{}));

		struct Result {
			Voxel voxel{};
			glm::ivec3 point;
			glm::ivec3 normal{};
			float distance{};
		};

		const auto raycast = [&](const glm::vec3 origin, const glm::vec3 direction, const float max_distance) -> std::optional<Result> {
			if (glm::dot(direction, direction) < 1e-8f) {
				return std::nullopt;
			}

			glm::ivec3 point = glm::floor(origin);
			const glm::ivec3 step = glm::sign(direction);

			const glm::vec3 inv_dir = 1.0f / glm::max(glm::abs(direction), glm::vec3(1e-8f));

			const glm::vec3 next_boundary = glm::vec3(point) +
				glm::vec3(
					step.x > 0 ? 1.0f : 0.0f,
					step.y > 0 ? 1.0f : 0.0f,
					step.z > 0 ? 1.0f : 0.0f
				);

			glm::vec3 t_max = (next_boundary - origin) * inv_dir;
			const glm::vec3 t_delta = glm::vec3(fabs(inv_dir.x), fabs(inv_dir.y), fabs(inv_dir.z));

			float distance = 0.0f;
			glm::ivec3 normal{0};

			while (distance <= max_distance) {
				if (!bounds.contains(point))
					break;

				if (const Voxel voxel_value = sampler(point); voxel_value.is_solid()) {
					return Result {
						.voxel = voxel_value,
						.distance = distance,
						.point = point,
						.normal = normal
					};
				}

				if (t_max.x < t_max.y && t_max.x < t_max.z) {
					point.x += step.x;
					distance = t_max.x;
					t_max.x += t_delta.x;
					normal = { -step.x, 0, 0 };
				}
				else if (t_max.y < t_max.z) {
					point.y += step.y;
					distance = t_max.y;
					t_max.y += t_delta.y;
					normal = { 0, -step.y, 0 };
				}
				else {
					point.z += step.z;
					distance = t_max.z;
					t_max.z += t_delta.z;
					normal = { 0, 0, -step.z };
				}
			}

			return std::nullopt;
		};

		return raycast;
	}
}

#endif //CYREX_VOXELS_RAY_H