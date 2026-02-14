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

static vox::VoxelMesh explosion(const float time) {
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

    const auto sampler = [time](const glm::ivec3 p) -> Voxel {
        glm::vec3 pos = glm::vec3(p);

        // ----------------------------------
        // Rotation
        // ----------------------------------

        float rot_speed = 0.8f;
        float angle_y = time * rot_speed;
        float angle_x = sin(time * 1.2f) * 0.3f; // subtle wobble

        glm::mat3 rotY = glm::mat3(
            glm::vec3( cos(angle_y), 0.0f, sin(angle_y)),
            glm::vec3( 0.0f,         1.0f, 0.0f        ),
            glm::vec3(-sin(angle_y), 0.0f, cos(angle_y))
        );

        glm::mat3 rotX = glm::mat3(
            glm::vec3(1.0f, 0.0f,          0.0f         ),
            glm::vec3(0.0f, cos(angle_x), -sin(angle_x)),
            glm::vec3(0.0f, sin(angle_x),  cos(angle_x))
        );

        // Apply rotation to sample space
        pos = rotY * rotX * pos;

        const float size = 8.0f;
        const float stage_duration = 3.0f;
        const float total_duration = stage_duration * 2.0f;

        float t_total = fmod(time, total_duration);

        // -------------------------------------------------
        // Bounce easing (overshoot)
        // -------------------------------------------------
        auto ease_out_back = [](float t) {
            const float c1 = 2.8f;      // increase for more bounce
            const float c3 = c1 + 1.0f;
            return 1.0f + c3 * std::pow(t - 1.0f, 3.0f)
                         + c1 * std::pow(t - 1.0f, 2.0f);
        };

        // -------------------------------------------------
        // SDF: Cube
        // -------------------------------------------------
        glm::vec3 half_extents(size);
        glm::vec3 q = glm::abs(pos) - half_extents;

        float sdf_cube =
            glm::length(glm::max(q, glm::vec3(0.0f))) +
            glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);

        // -------------------------------------------------
        // SDF: Sphere
        // -------------------------------------------------
        float sdf_sphere = glm::length(pos) - size;

        // -------------------------------------------------
        // SDF: Torus (around Y axis)
        // -------------------------------------------------
        float sdf_torus;
        {
            float R = size * 0.7f;  // major radius
            float r = size * 0.3f;  // minor radius

            glm::vec2 tpos(
                glm::length(glm::vec2(pos.x, pos.z)) - R,
                pos.y
            );

            sdf_torus = glm::length(tpos) - r;
        }

        float sdf = 0.0f;
        float t = 0.0f;

        // -------------------------------------------------
        // Stage Selection
        // -------------------------------------------------
        if (t_total < stage_duration) {
            // Cube → Sphere
            float local = t_total / stage_duration;
            t = ease_out_back(local);

            sdf = glm::mix(sdf_cube, sdf_sphere, t);
        }
        else {
            // Sphere → Torus
            float local = (t_total - stage_duration) / stage_duration;
            t = ease_out_back(local);

            sdf = glm::mix(sdf_sphere, sdf_torus, t);
        }

        Voxel v{};

        if (sdf < 0.0f) {
            v.solid = true;

            glm::vec3 blue   = {0.1f, 0.3f, 1.0f};
            glm::vec3 red    = {1.0f, 0.1f, 0.1f};
            glm::vec3 purple = {0.7f, 0.2f, 0.9f};

            if (t_total < stage_duration)
                v.color = glm::mix(blue, red, glm::clamp(t, 0.0f, 1.0f));
            else
                v.color = glm::mix(red, purple, glm::clamp(t, 0.0f, 1.0f));
        }

        v.color -= hash(p) * 0.1f;

        return v;
    };

    const auto mesher = vox::make_blocky_mesher<decltype(sampler)>({
        .from = {-20, -20, -20},
        .to   = { 20,  20,  20},
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

    struct Frame {
        vox::VoxelMesh mesh;
        gfx::VertexBuffer vbo;
        gfx::IndexBuffer ibo;
        gfx::VertexArray vao;

        constexpr explicit Frame(
            const vox::VoxelMesh& mesh,
            gfx::VertexBuffer&& vbo,
            gfx::IndexBuffer&& ibo,
            gfx::VertexArray&& vao
        ) : mesh(mesh), vbo(std::move(vbo)), ibo(std::move(ibo)), vao(std::move(vao)) {}
    };

    // generate the frames :)
    std::vector<Frame> frames;
    for (float t = 0.0f; t < 6.0f; t += 0.1f) {
        const auto mesh = explosion(t);
        const auto& [vertices, indices] = mesh;
        auto vbo = gfx::VertexBuffer::make_fixed(std::span(vertices));
        auto ibo = gfx::IndexBuffer::make_fixed(std::span(indices));
        auto vao = gfx::VertexArray::make_voxel(vbo, ibo);

        frames.emplace_back(
            mesh,
            std::move(vbo),
            std::move(ibo),
            std::move(vao) );
    }


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

        float t = glfwGetTime() * 25.0f;
        int count = static_cast<int>(frames.size());
        int period = count * 2 - 2;

        int i = static_cast<int>(fmodf(t, period));
        i = count - 1 - std::abs(i - (count - 1));

        const auto& frame = frames[i];


        gfx::VertexArray::bind(frame.vao);
        gfx::draw_triangles_indexed<gfx::IndexType::Uint>({0, static_cast<GLuint>(frame.mesh.indices.size())});

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}