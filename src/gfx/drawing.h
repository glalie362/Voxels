//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_DRAWING_H
#define VOXEL_GAME_DRAWING_H
#include <cassert>

#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"
#include "glbinding/gl/types.h"

namespace gfx {
    struct DrawRange {
        gl::GLuint begin_vertex{};
        gl::GLuint end_vertex{};

        constexpr DrawRange(const gl::GLuint begin, const gl::GLuint end) :
            begin_vertex{begin}, end_vertex{end} {
            assert(begin <= end);
        }

        [[nodiscard]] constexpr gl::GLuint count() const noexcept { return end_vertex - begin_vertex; }
    };

    void draw_triangles_array(DrawRange range);

    enum class IndexType {
        Ubyte,
        Ushort,
        Uint,
        Ulong,
    };

    template<IndexType index_type>
    void draw_triangles_indexed(const DrawRange range) {
        const auto ty = [] (const IndexType it) {
            switch (it) {
                case IndexType::Ubyte: return gl::GL_UNSIGNED_BYTE;
                case IndexType::Ushort: return gl::GL_UNSIGNED_SHORT;
                case IndexType::Uint: return gl::GL_UNSIGNED_INT;
                case IndexType::Ulong: return gl::GL_UNSIGNED_INT64_ARB;
            }
            std::unreachable();
        };
        gl::glDrawElements(gl::GLenum::GL_TRIANGLES, range.count(), ty(index_type), nullptr);
    }
}

#endif //VOXEL_GAME_DRAWING_H