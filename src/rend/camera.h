//
// Created by Jamie on 12/02/2026.
//

#ifndef VOXEL_GAME_CAMERA_H
#define VOXEL_GAME_CAMERA_H
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace rend {
    using Position = glm::vec3;
    using Rotation = glm::quat;

    using Degrees = float;
    using Distance = float;

    struct Transform {
        Position position{};
        Rotation rotation{};

        [[nodiscard]] static glm::mat4 make_matrix(const Transform& transform);
    };

    struct RenderMatrices {
        glm::mat4 projection{};
        glm::mat4 view{};

        [[nodiscard]] constexpr static RenderMatrices identity() {
            return {
                .projection = glm::identity<glm::mat4>(),
                .view = glm::identity<glm::mat4>(),
            };
        }
    };

    struct FirstPersonCamera {
        Position eye{};
        Degrees yaw{};
        Degrees pitch{};
        Degrees field_of_view{60.0f};
        Distance near_clip{0.1f};
        Distance far_clip{100.0f};
        glm::vec2 screen {1.0f, 1.0f};

        [[nodiscard]] static Transform make_transform(const FirstPersonCamera& camera) noexcept;
        [[nodiscard]] static RenderMatrices make_render_matrices(const FirstPersonCamera& camera) noexcept;
    };



}

#endif //VOXEL_GAME_CAMERA_H