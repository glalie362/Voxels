//
// Created by Jamie on 11/02/2026.
//

#include "vertex_array.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"

#include <glm/glm.hpp>

static gl::GLuint make_vao() {
    gl::GLuint vertex_array;
    gl::glCreateVertexArrays(1, &vertex_array);
    return vertex_array;
}

namespace {
    struct Attribute {
        gl::GLuint index{};
        gl::GLuint offset{};
        gl::GLuint size{};
        gl::GLenum type{};
    };
}


template<typename Vertex>
static gl::GLuint with_attribute(const gl::GLuint vao, const gfx::VertexBuffer& buffer, const Attribute attribute) {
    gl::glEnableVertexArrayAttrib(vao, attribute.index);
    gl::glVertexArrayVertexBuffer(vao, attribute.index, buffer.handle(), attribute.offset, sizeof(Vertex));
    gl::glVertexArrayAttribFormat(vao, attribute.index, attribute.size, attribute.type, gl::GL_FALSE, 0);
    gl::glVertexArrayAttribBinding(vao, attribute.index, attribute.index);
    return vao;
}

gfx::VertexArray gfx::VertexArray::make_testing(const VertexBuffer& vertex_buffer, const IndexBuffer& index_buffer) {
    enum {
        Position,
        Color
    };

    VertexArray vao{make_vao()};
    vao.vertex_array = with_attribute<VertexTesting>(vao.vertex_array, vertex_buffer, Attribute{
        .index = Position,
        .offset = offsetof(VertexTesting, xyz),
        .size = 3,
        .type = gl::GLenum::GL_FLOAT
    });
    vao.vertex_array = with_attribute<VertexTesting>(vao.vertex_array, vertex_buffer, Attribute{
        .index = Color,
        .offset = offsetof(VertexTesting, rgb),
        .size = 3,
        .type = gl::GLenum::GL_FLOAT
    });

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
