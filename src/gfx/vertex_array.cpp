//
// Created by Jamie on 11/02/2026.
//

#include "vertex_array.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"

#include <glm/glm.hpp>
#include "../vox/voxel.h"

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

gfx::VertexArray gfx::VertexArray::make_voxel(const VertexBuffer& vertex_buffer, const IndexBuffer& index_buffer) {
    enum {
        Position,
        Normal,
        Color
    };

    using Vertex = vox::VoxelMesh::Vertex;

    VertexArray vao{make_vao()};
    vao.vertex_array = with_attribute<Vertex>(vao.vertex_array, vertex_buffer, Attribute{
        .index = Position,
        .offset = offsetof(Vertex, position),
        .size = 3,
        .type = gl::GLenum::GL_FLOAT
    });

    vao.vertex_array = with_attribute<Vertex>(vao.vertex_array, vertex_buffer, Attribute{
        .index = Normal,
        .offset = offsetof(Vertex, normal),
        .size = 3,
        .type = gl::GLenum::GL_FLOAT
    });

    vao.vertex_array = with_attribute<Vertex>(vao.vertex_array, vertex_buffer, Attribute{
        .index = Color,
        .offset = offsetof(Vertex, color),
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
