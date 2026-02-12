#include <glbinding/glbinding.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

#include "gfx/vertex_array.h"
#include "gfx/vertex_buffer.h"
#include "gfx/index_buffer.h"
#include "gfx/drawing.h"
#include "gfx/shader.h"

#include <glm/vec3.hpp>


constexpr static std::string_view test_vertex_source = R"(
#version 460 core

in vec3 in_position;
in vec3 in_color;

out vec3 color;

void main() {
  color = in_color;
  gl_Position = vec4(in_position, 1.0);
}
)";

constexpr static std::string_view test_fragment_source = R"(
#version 460 core

in vec3 color;

out vec4 frag;

void main() {
  frag = vec4(cdasolor, 1.0);
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

  using glm::vec3;
  using xyz = vec3;
  using rgb = vec3;

  const auto vbo = gfx::VertexBuffer::make_fixed(std::array{
    xyz(-1.0f, -1.0f, 0.0f), rgb(1.0f, 0.0f, 0.0f),
    xyz(1.0f, -1.0f, 0.0f),  rgb(0.0f, 1.0f, 0.0f),
    xyz(0.0f, 1.0f, 0.0f),  rgb(0.0f, 0.0f, 1.0f),
  });

  const auto ibo = gfx::IndexBuffer::make_fixed(std::array{0, 1, 2});
  const auto vao = gfx::VertexArray::make_testing(vbo, ibo);
  gfx::VertexArray::bind(vao);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    gfx::draw_triangles_indexed<gfx::IndexType::Uint>({0, 3});
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
