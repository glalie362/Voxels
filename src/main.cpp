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

#include "cyrex_voxels/vox/cache.h"
#include "cyrex_voxels/vox/marching.h"

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
    float light = 0.5 + 0.5 * dot(normal, normalize(vec3(0.5, 0.75, 0.1)));
    light = max(light, 0.1);
    frag = vec4(color * light, 1.0);
}
)";

enum class MyVoxel {
    None,
    Solid
};

auto hash01(const vox::Coord coord) {
    return static_cast<float>(std::hash<vox::Coord>{}(coord) & 0xFFFF) / static_cast<float>(0xFFFF);
}

template<>
struct vox::voxel_mesh_traits<MyVoxel> {
    [[nodiscard]] constexpr static bool is_visible(const MyVoxel voxel) {
        return voxel != MyVoxel::None;
    }

    static glm::vec3 rainbow(const Coord coord) {
        const float pi = glm::pi<float>();
        const float shift = glm::radians(120.0f);
        const float radians = glm::radians(float(coord.y)) * pi;

        return glm::vec3(0.5f) + glm::vec3(0.5f) *
            glm::sin(glm::vec3(radians, radians + shift, radians - shift));
    }

    [[nodiscard]] constexpr static Color color(const MyVoxel voxel, const Coord coord) {
        switch (voxel) {
            case MyVoxel::None: return {0.0f, 0.0f, 0.0f, 0.0f};
            case MyVoxel::Solid: return {rainbow(coord) - glm::vec3(hash01(coord) * 0.1f), 1.0f};
        }
    }
};

template<>
struct vox::voxel_convert_traits<bool, MyVoxel> {
    [[nodiscard]] constexpr MyVoxel operator()(const bool voxel) const {
        return voxel ? MyVoxel::Solid : MyVoxel::None;
    }
};


enum class TerrainVoxel {
    None,
    Grass,
    Sand,
    Dirt,
    Wood,
    Leaves
};

[[nodiscard]] constexpr TerrainVoxel operator | (const TerrainVoxel lhs, const TerrainVoxel rhs) {
    if (lhs == TerrainVoxel::None) return rhs;
    if (rhs == TerrainVoxel::None) return lhs;
    return rhs;
}

template<>
struct vox::voxel_mesh_traits<TerrainVoxel> {
    [[nodiscard]] constexpr static bool is_visible(const TerrainVoxel voxel) {
        return voxel != TerrainVoxel::None;
    }

    [[nodiscard]] constexpr static Color color(const TerrainVoxel voxel, const Coord coord) {
        switch (voxel) {
            case TerrainVoxel::None: return {0.0f, 0.0f, 0.0f, 0.0f};
            case TerrainVoxel::Grass: return Color(0.2f, 0.8f, 0.1f, 1.0f) - hash01(coord) * 0.1f;
            case TerrainVoxel::Sand: return {1.0f, 0.9f, 0.5f, 1.0f};
            case TerrainVoxel::Dirt: return {0.6f, 0.4f, 0.4f, 1.0f};
            case TerrainVoxel::Wood: return {0.3f, 0.2f, 0.1f, 1.0f};
            case TerrainVoxel::Leaves: return {0.0f, 0.4f, 0.0f, 1.0f};
        }
    }
};


#include <chrono>

constexpr static vox::VoxelMesh test() {
    using namespace vox;

    const int size = 256;

    constexpr auto world_bounds = Bounds{
        .from = {-size, -64, -size},
        .to   = {size, 64, size},
    };

    const auto heightmap = flat_cache([&](const Coord coord) {
        return 0.5f + 0.5f * glm::perlin(glm::vec2(coord.x, coord.z) * 0.0125f) * 30.0f;
    }, {{-size, 0, -size}, {size, 0, size}});

    const auto sphere_cutout = flat_cache(sphere(Coord(), 10), cube_bounds(20));

    const auto cut = [](const auto left, const auto right) {
        return [=](const Coord& coord) {
            const auto a = left(coord);
            const auto b = right(coord);
            if (b) return decltype(a){};
            return a;
        };
    };

    const auto terrain_sampler = [&](const Coord coord) {
        const int diff = coord.y - heightmap(Coord(coord.x, 0, coord.z));
        if (diff > 0) return TerrainVoxel::None;
        if (diff == 0) { return coord.y < -8 ? TerrainVoxel::Sand : TerrainVoxel::Grass; }
        return TerrainVoxel::Dirt;
    };

    const auto cutout_sampler = flat_cache(
        transform{}
        // << translate({10, 0, 0})
        // << repeat({50, 0, 50})
        << sphere_cutout,
        world_bounds);

    const auto start = std::chrono::high_resolution_clock::now();
    const auto big_cache = flat_cache(cut(terrain_sampler, cutout_sampler), world_bounds);
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "big_cache build time: " << duration.count() << " ms\n";
    const auto mesher = make_marching_mesher(big_cache);
    return mesher(world_bounds);
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
