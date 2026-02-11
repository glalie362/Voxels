//
// Created by Jamie on 11/02/2026.
//

#include "vertex_array.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"

#include <glm/glm.hpp>

gfx::VertexArray gfx::VertexArray::make_testing(const VertexBuffer& vertex_buffer, const IndexBuffer& index_buffer) {
    VertexArray vao{};
    gl::glCreateVertexArrays(1, &vao.vertex_array);
    gl::glEnableVertexArrayAttrib(vao.vertex_array, 0);
    gl::glVertexArrayVertexBuffer(vao.vertex_array, 0, vertex_buffer.handle(), 0, sizeof(glm::vec3));
    gl::glVertexArrayAttribFormat(vao.vertex_array, 0, 3, gl::GLenum::GL_FLOAT, gl::GL_FALSE, 0);
    gl::glVertexArrayAttribBinding(vao.vertex_array, 0, 0);
    gl::glVertexArrayElementBuffer(vao.vertex_array, index_buffer.handle());
    return vao;
}

void gfx::VertexArray::bind(const VertexArray &vao) {
    gl::glBindVertexArray(vao.vertex_array);
}

gfx::VertexArray::~VertexArray() {
    gl::glDeleteVertexArrays(1, &vertex_array);
}

gfx::VertexArray::VertexArray(VertexArray&& temp) {
    vertex_array = temp.vertex_array;
    temp.vertex_array = 0;
}

gfx::VertexArray & gfx::VertexArray::operator=(VertexArray&& temp) {
    gl::glDeleteVertexArrays(1, &vertex_array);
    vertex_array = temp.vertex_array;
    temp.vertex_array = 0;
    return *this;
}
