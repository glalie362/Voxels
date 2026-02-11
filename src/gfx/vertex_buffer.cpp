//
// Created by Jamie on 11/02/2026.
//

#include "vertex_buffer.h"

#include "glbinding/gl/bitfield.h"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"

gfx::VertexBuffer gfx::VertexBuffer::make_empty_fixed_size(const std::size_t num_bytes) {
    VertexBuffer vbo{};
    gl::glCreateBuffers(1, &vbo.vertex_buffer);
    gl::glNamedBufferStorage(vbo.vertex_buffer, num_bytes, nullptr, gl::BufferStorageMask::GL_DYNAMIC_STORAGE_BIT);
    return vbo;
}

void gfx::VertexBuffer::bind() const {
    gl::glBindBuffer(gl::GLenum::GL_ARRAY_BUFFER, vertex_buffer);
}

gfx::VertexBuffer::~VertexBuffer() {
    gl::glDeleteBuffers(1, &vertex_buffer);
}

gfx::VertexBuffer::VertexBuffer(VertexBuffer&& temp) {
    vertex_buffer = temp.vertex_buffer;
    temp.vertex_buffer = 0;
}

gfx::VertexBuffer& gfx::VertexBuffer::operator=(VertexBuffer&& temp) {
    gl::glDeleteBuffers(1, &vertex_buffer);
    vertex_buffer = temp.vertex_buffer;
    temp.vertex_buffer = 0;
    return *this;
}
