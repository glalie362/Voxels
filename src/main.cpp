#define GLM_ENABLE_EXPERIMENTAL
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

#include <glm/gtx/hash.hpp>

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
    float light = max(0.3, dot(normal, normalize(vec3(0.4, 0.95, 0.65))));
    frag = vec4(color * light, 1.0);
}
)";

static float hash(const glm::ivec3& p) {
    uint32_t h =
        uint32_t(p.x) * 374761393u ^
        uint32_t(p.y) * 668265263u ^
        uint32_t(p.z) * 2147483647u;

    h = (h ^ (h >> 13)) * 1274126177u;
    return float(h & 0x00FFFFFF) / float(0x00FFFFFF); // 0..1
}

static vox::VoxelMesh test(){
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

    constexpr static int W = 32;
    constexpr static int H = 32;
    constexpr static int D = 32;
    Voxel voxels[W][H][D] {};

    for (int x = 0; x < W; ++x) {
        for (int z = 0; z < D; ++z) {
            voxels[x][0][z] = {.solid = true, .color = {0.5f, 0.5f, 0.5f}};
            voxels[x][1][z] = {.solid = true, .color = {0.5f, 0.5f, 0.5f}};
            voxels[x][2][z] = {.solid = true, .color = {0.5f, 0.5f, 0.5f}};
            voxels[x][3][z] = {.solid = true, .color = {0.4f, 0.3f, 0.3f}};
            voxels[x][4][z] = {.solid = true, .color = {0.4f, 0.3f, 0.3f}};
            voxels[x][5][z] = {.solid = true, .color = {0.3f, 0.8f, 0.2f}};
        }
    }

    const auto sampler = [&](const glm::ivec3 p) -> Voxel {
        return voxels[p.x][p.y][p.z];
    };

    const auto mesher = vox::make_blocky_mesher<decltype(sampler)>({
        .from = {0, 0, 0},
        .to   = { W,  H,  D},
    });

    return mesher(sampler);
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
    auto vbo = gfx::VertexBuffer::make_fixed(std::span(vertices));
    auto ibo = gfx::IndexBuffer::make_fixed(std::span(indices));
    auto vao = gfx::VertexArray::make_voxel(vbo, ibo);

    rend::FirstPersonCamera camera;
    camera.eye    = {0.0f, 0.0f, 20.0f};
    camera.screen = {1920.0f, 1080.0f};
    camera.yaw = 45.0f;
    camera.far_clip = 500.0f;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.5f, 0.65f, 1.0f, 1.0f);

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