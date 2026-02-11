#include <glbinding/glbinding.h>
#include <glbinding/gl/functions.h>
#include "gfx/vertex_array.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "gfx/vertex_buffer.h"
#include "gfx/index_buffer.h"
#include <glm/vec3.hpp>

#include "gfx/drawing.h"


int main() {
  using namespace gl;

  if (glfwInit() == 0) {
    return 1;
  }

  GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);
  if (!window) {
    return 1;
  }

  glfwMakeContextCurrent(window);
  glbinding::initialize(glfwGetProcAddress);

  using glm::vec3;

  auto vbo = gfx::VertexBuffer::make_fixed(std::array{
    vec3(-1.0f, -1.0f, 0.0f),
    vec3(1.0f, -1.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f)
  });
  auto ibo = gfx::IndexBuffer::make_fixed(std::array{0, 1, 2});

  auto vao = gfx::VertexArray::make_testing(vbo, ibo);
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
