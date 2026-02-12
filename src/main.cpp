#include <glbinding/glbinding.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

#include "gfx/vertex_array.h"
#include "gfx/vertex_buffer.h"
#include "gfx/index_buffer.h"
#include "gfx/drawing.h"
#include "gfx/shader.h"

#include "rend/camera.h"

constexpr static std::string_view test_vertex_source = R"(
#version 460 core

uniform mat4 projection;
uniform mat4 view;

in vec3 in_position;
in vec3 in_color;

out vec3 color;

void main() {
  color = in_color;
  gl_Position = projection * view * vec4(in_position, 1.0);
}
)";

constexpr static std::string_view test_fragment_source = R"(
#version 460 core

in vec3 color;

out vec4 frag;

void main() {
  frag = vec4(color, 1.0);
}
)";

int main() {
  using namespace gl;

  if (glfwInit() == 0) {
    return 1;
  }

  GLFWwindow *window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);
  if (!window) {
    return 1;
  }

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

  using glm::vec3;
  using xyz = vec3;
  using rgb = vec3;

  const auto vbo = gfx::VertexBuffer::make_fixed(std::array{
    // front face
    xyz(-1.0f, -1.0f,  1.0f), rgb(1.0f, 0.0f, 0.0f),
    xyz( 1.0f, -1.0f,  1.0f), rgb(0.0f, 1.0f, 0.0f),
    xyz( 1.0f,  1.0f,  1.0f), rgb(0.0f, 0.0f, 1.0f),
    xyz(-1.0f,  1.0f,  1.0f), rgb(1.0f, 1.0f, 0.0f),

    // back face
    xyz(-1.0f, -1.0f, -1.0f), rgb(1.0f, 0.0f, 1.0f),
    xyz( 1.0f, -1.0f, -1.0f), rgb(0.0f, 1.0f, 1.0f),
    xyz( 1.0f,  1.0f, -1.0f), rgb(1.0f, 1.0f, 1.0f),
    xyz(-1.0f,  1.0f, -1.0f), rgb(0.3f, 0.3f, 0.3f),
  });

  const auto ibo = gfx::IndexBuffer::make_fixed(std::array{
    // front
    0, 1, 2,  2, 3, 0,
    // right
    1, 5, 6,  6, 2, 1,
    // back
    5, 4, 7,  7, 6, 5,
    // left
    4, 0, 3,  3, 7, 4,
    // top
    3, 2, 6,  6, 7, 3,
    // bottom
    4, 5, 1,  1, 0, 4,
  });

  const auto vao = gfx::VertexArray::make_testing(vbo, ibo);
  gfx::VertexArray::bind(vao);

  rend::FirstPersonCamera camera;
  camera.eye = {0.0f, 0.0f, 5.0f};
  camera.screen = {800.0f, 600.0f};

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int num_joysticks{};
    const float* axis = glfwGetJoystickAxes(0, &num_joysticks);
    if (num_joysticks != 0) {
      camera.yaw += axis[GLFW_GAMEPAD_AXIS_RIGHT_X];
      camera.pitch += axis[GLFW_GAMEPAD_AXIS_RIGHT_Y];
    }

    const auto [projection, view] = rend::FirstPersonCamera::make_render_matrices(camera);
    program->uniform_matrix("projection", projection);
    program->uniform_matrix("view", view);

    gfx::draw_triangles_indexed<gfx::IndexType::Uint>({0, 36});
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
