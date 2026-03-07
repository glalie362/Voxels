#define GLM_ENABLE_EXPERIMENTAL
#include <glbinding/glbinding.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <span>
#include <random>

// gpu
#include <cyrex_voxels/gfx/vertex_array.h>
#include <cyrex_voxels/gfx/vertex_buffer.h>
#include <cyrex_voxels/gfx/index_buffer.h>
#include <cyrex_voxels/gfx/drawing.h>
#include <cyrex_voxels/gfx/shader.h>

// rendering
#include <cyrex_voxels/rend/camera.h>


// voxel library
#include <cyrex_voxels/vox/transform.h>
#include <cyrex_voxels/vox/convert.h>
#include <cyrex_voxels/vox/pipeline.h>
#include <cyrex_voxels/vox/samplers.h>
#include <cyrex_voxels/vox/blocky.h>

#include <glm/gtx/hash.hpp>

#include <cyrex_voxels/vox/cache.h>
#include <cyrex_voxels/vox/marching.h>

constexpr std::string_view test_vertex_source = R"(
#version 460 core

uniform mat4 projection;
uniform mat4 view;

in vec3 in_position;
in vec3 in_normal;
in vec3 in_color;

out vec3 normal;
out vec3 color;

void main()
{
    normal = in_normal;
    color  = in_color;
    gl_Position = projection * view * vec4(in_position, 1.0);
}
)";

constexpr std::string_view test_fragment_source = R"(
#version 460 core

in vec3 normal;
in vec3 color;

out vec4 frag;

void main()
{
    float light = 0.5 + 0.5 * dot(normal, normalize(vec3(0.2, 0.75, 0.12)));
    light = max(light, 0.4);
    frag = vec4(color * light, 1.0);
}
)";

constexpr static vox::VoxelMesh test() {
    using namespace vox;
    return {};
}

int main() {
    using namespace gl;
    if (!glfwInit()) return 1;
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Hello World", nullptr, nullptr);
    if (!window) return 1;

    glfwMakeContextCurrent(window);
    glbinding::initialize(glfwGetProcAddress);

    const auto program = gfx::Shader::from_source(
        test_vertex_source,
        test_fragment_source
    );

    if (!program) {
        std::cerr << program.error() << '\n';
        return 1;
    }

    gfx::Shader::bind(*program);

    const auto& [vertices, indices] = test();
    const auto vbo = gfx::VertexBuffer::make_fixed(std::span(vertices));
    const auto ibo = gfx::IndexBuffer::make_fixed(std::span(indices));
    const auto vao = gfx::VertexArray::make_voxel(vbo, ibo);

    rend::FirstPersonCamera camera;
    camera.eye    = {0.0f, 0.0f, 20.0f};
    camera.screen = {1920.0f, 1080.0f};
    camera.yaw = 45.0f;
    camera.far_clip = 500.0f;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.3f, 0.7f, 1.0f, 1.0f);

    // glEnable(GL_CULL_FACE);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int axis_count{};
        const float* axis = glfwGetJoystickAxes(0, &axis_count);

        if (axis && axis_count > 0) {
            camera.yaw   += axis[GLFW_GAMEPAD_AXIS_RIGHT_X] * 2.0f;
            camera.pitch += axis[GLFW_GAMEPAD_AXIS_RIGHT_Y] * 2.0f;

            const auto [position, rotation] = rend::FirstPersonCamera::make_transform(camera);
            camera.eye += glm::vec3(0.0f, 0.0f, 1.0f) * rotation * axis[GLFW_GAMEPAD_AXIS_LEFT_Y];
            camera.eye += glm::vec3(1.0f, 0.0f, 0.0f) * rotation * axis[GLFW_GAMEPAD_AXIS_LEFT_X];
        }

        const auto [projection, view] = rend::FirstPersonCamera::make_render_matrices(camera);
        program->uniform_matrix("projection", projection);
        program->uniform_matrix("view", view);

        gfx::VertexArray::bind(vao);
        gfx::draw_triangles_indexed<gfx::IndexType::Uint>({0, static_cast<GLuint>(indices.size())});

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
