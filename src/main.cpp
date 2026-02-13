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
    float light = max(0.3, dot(normal, normalize(vec3(0.4, 2.0, 0.6))));
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



    const auto sampler = [](const glm::ivec3 p) -> Voxel {
        Voxel v{};

        const glm::vec3 pos = glm::vec3(p);

        // --- Floating island base (sphere falloff) ---
        const float islandRadius = 35.0f;
        const float d = glm::length(pos);

        if (d > islandRadius)
            return v; // empty space

        // --- Height-based terrain deformation ---
        const float heightNoise =
            6.0f * sinf(pos.x * 0.15f) * cosf(pos.z * 0.15f) +
            3.0f * sinf(pos.x * 0.05f + pos.z * 0.05f);

        const float terrainHeight = 10.0f + heightNoise;

        if (pos.y > terrainHeight)
            return v;

        // --- Carve caves ---
        const float caveNoise =
            sinf(pos.x * 0.2f) *
            sinf(pos.y * 0.2f) *
            sinf(pos.z * 0.2f);

        if (caveNoise > 0.6f)
            return v;

        v.solid = true;

        // --- Color logic ---
        if (pos.y > terrainHeight - 1.5f) {
            // Grass
            v.color = {0.2f, 0.8f, 0.3f};
        }
        else if (pos.y > -5.0f) {
            // Dirt
            v.color = {0.45f, 0.3f, 0.15f};
        }
        else {
            // Stone
            v.color = {0.5f, 0.5f, 0.55f};
        }

        // --- Glowing core in the center ---
        if (glm::length(pos) < 6.0f) {
            v.color = {1.0f, 0.3f, 0.1f}; // lava core
        }

        v.color -= hash(pos) * 0.1f;

        return v;
    };

    const auto mesher = vox::make_blocky_mesher<decltype(sampler)>({
        .from = {-50, -50, -50},
        .to   = { 50,  50,  50},
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

    const auto [vertices, indices] = test();
    const auto vbo = gfx::VertexBuffer::make_fixed(std::span(vertices));
    const auto ibo = gfx::IndexBuffer::make_fixed(std::span(indices));
    const auto vao = gfx::VertexArray::make_voxel(vbo, ibo);

    gfx::VertexArray::bind(vao);

    rend::FirstPersonCamera camera;
    camera.eye    = {5.0f, 30.0f, 12.6f};
    camera.screen = {1920.0f, 1080.0f};
    camera.yaw = 45.0f;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.5f, 0.65f, 1.0f, 1.0f);

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