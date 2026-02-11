//
// Created by Jamie on 11/02/2026.
//

#include "drawing.h"

#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"

void gfx::draw_triangles_array(const DrawRange range) {
    gl::glDrawArrays(gl::GLenum::GL_TRIANGLES, range.begin_vertex, range.count());
}

