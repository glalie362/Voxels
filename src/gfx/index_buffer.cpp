//
// Created by Jamie on 11/02/2026.
//

#include "index_buffer.h"

#include "glbinding/gl/enum.h"

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