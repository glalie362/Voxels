#include <glbinding/glbinding.h>
#include <glbinding/gl/functions.h>
#include "gfx/vertex_array.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "gfx/vertex_buffer.h"
#include "glbinding/gl/enum.h"
#include "glm/vec3.hpp"

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
  vbo.bind();

  auto vao = gfx::VertexArray::make_testing(vbo);
  vao.bind();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    vbo.draw_triangles({0u, 3u});
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
