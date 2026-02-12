//
// Created by Jamie on 11/02/2026.
//

#include "index_buffer.h"

#include "glbinding/gl/enum.h"

gl::GLuint gfx::detail::make_gl_index_buffer() {
    gl::GLuint index_buffer{};
    gl::glCreateBuffers(1, &index_buffer);
    return index_buffer;
}

void gfx::IndexBuffer::bind(const IndexBuffer& ibo) {
    gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, ibo.index_buffer);
}

gfx::IndexBuffer::~IndexBuffer() {
    gl::glDeleteBuffers(1, &index_buffer);
}

gfx::IndexBuffer::IndexBuffer(IndexBuffer&& temp) {
    index_buffer = temp.index_buffer;
    temp.index_buffer = 0;
}

gfx::IndexBuffer& gfx::IndexBuffer::operator=(IndexBuffer&& temp) {
    gl::glDeleteBuffers(1, &index_buffer);
    index_buffer = temp.index_buffer;
    temp.index_buffer = 0;
    return *this;
}