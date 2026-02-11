//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_VERTEX_BUFFER_H
#define VOXEL_GAME_VERTEX_BUFFER_H
#include <cassert>
#include <span>

#include <glbinding/gl/types.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

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

    class VertexBuffer {
    public:
        [[nodiscard]] static VertexBuffer make_empty_fixed_size(std::size_t num_bytes);

        template<typename T>
        [[nodiscard]] static VertexBuffer make_fixed(std::span<const T> data);

        template<typename T, std::size_t N>
        [[nodiscard]] static VertexBuffer make_fixed(const std::array<T, N>& arr) {
            return make_fixed<T>(std::span{arr});
        }

        void bind() const;
        void draw_triangles(DrawRange range) const;

        [[nodiscard]] constexpr gl::GLuint handle() const noexcept {
            return vertex_buffer;
        }

        ~VertexBuffer();
        VertexBuffer(VertexBuffer const&) = delete;
        VertexBuffer& operator=(VertexBuffer const&) = delete;
        VertexBuffer(VertexBuffer&&);
        VertexBuffer& operator=(VertexBuffer&&);
    private:
        VertexBuffer() = default;
        gl::GLuint vertex_buffer{};
    };

    template<typename T>
    VertexBuffer VertexBuffer::make_fixed(const std::span<const T> data) {
        VertexBuffer vbo{};
        gl::glCreateBuffers(1, &vbo.vertex_buffer);
        gl::glNamedBufferStorage(vbo.vertex_buffer,
            data.size_bytes(),
            data.data(),
            gl::BufferStorageMask::GL_DYNAMIC_STORAGE_BIT);
        return vbo;
    }
}

#endif //VOXEL_GAME_VERTEX_BUFFER_H