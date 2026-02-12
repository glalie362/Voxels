//
// Created by Jamie on 12/02/2026.
//

#include "camera.h"

#include "glm/ext/matrix_clip_space.hpp"

glm::mat4 rend::Transform::make_matrix(const Transform &transform) {
    const auto rotation_matrix = glm::mat4_cast(transform.rotation);
    const auto translation_matrix = glm::translate(glm::identity<glm::mat4>(), -transform.position);
    return rotation_matrix * translation_matrix;
}

rend::Transform rend::FirstPersonCamera::make_transform(const FirstPersonCamera &camera) noexcept {
    const auto yaw = glm::angleAxis(
        glm::radians(camera.yaw),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    const auto pitch = glm::angleAxis(
       glm::radians(camera.pitch),
       glm::vec3(1.0f, 0.0f, 0.0f)
    );

    const auto orientation = glm::normalize(pitch * yaw);

    return {
        .position = camera.eye,
        .rotation = orientation
    };
}

rend::RenderMatrices rend::FirstPersonCamera::make_render_matrices(const FirstPersonCamera &camera) noexcept {
    return {
        .projection = glm::perspectiveFov(
            glm::radians(camera.field_of_view),
            camera.screen.x,
            camera.screen.y,
            camera.near_clip,
            camera.far_clip
        ),
        .view = Transform::make_matrix(make_transform(camera))
    };
}
