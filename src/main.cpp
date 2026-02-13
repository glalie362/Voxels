#include <glbinding/glbinding.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <span>

#include "gfx/vertex_array.h"
#include "gfx/vertex_buffer.h"
#include "gfx/index_buffer.h"
#include "gfx/drawing.h"
#include "gfx/shader.h"

#include "vox/blocky.h"
#include "rend/camera.h"

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
    color  = in_color;
    normal = in_normal;
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
    frag = vec4(color, 1.0);
}
)";

static vox::VoxelMesh test() {
    struct Voxel {
        bool solid{};
        glm::vec3 color{1.0f};

        [[nodiscard]] constexpr bool is_solid() const noexcept {
            return solid;
        }

        [[nodiscard]] constexpr glm::vec3 colorized() const noexcept {
            return color;
        }

    };

    const auto sampler = [](glm::ivec3 p) -> Voxel {
        return {
            .solid = (p == glm::ivec3{0, 0, 0}),
            .color = {1.0f, 0.0f, 0.0f}
        };
    };

    const auto mesher = vox::make_blocky_mesher<decltype(sampler)>({
        .from = {-10, -10, -10},
        .to   = { 10,  10,  10},
    });

    return mesher(sampler);
}

int main()
{
    using namespace gl;

    if (!glfwInit())
        return 1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);
    if (!window)
        return 1;

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

    const auto [vertices, indices] = test();
    const auto vbo = gfx::VertexBuffer::make_fixed(std::span(vertices));
    const auto ibo = gfx::IndexBuffer::make_fixed(std::span(indices));
    const auto vao = gfx::VertexArray::make_voxel(vbo, ibo);

    gfx::VertexArray::bind(vao);

    rend::FirstPersonCamera camera;
    camera.eye    = {0.0f, 0.0f, 5.0f};
    camera.screen = {800.0f, 600.0f};

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.8f, 0.95f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int axis_count{};
        const float* axis = glfwGetJoystickAxes(0, &axis_count);

        if (axis && axis_count > 0) {
            camera.yaw   += axis[GLFW_GAMEPAD_AXIS_RIGHT_X];
            camera.pitch += axis[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        }

        const auto [projection, view] = rend::FirstPersonCamera::make_render_matrices(camera);

        program->uniform_matrix("projection", projection);
        program->uniform_matrix("view", view);
        gfx::draw_triangles_indexed<gfx::IndexType::Uint>({0, static_cast<GLuint>(indices.size())});

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}