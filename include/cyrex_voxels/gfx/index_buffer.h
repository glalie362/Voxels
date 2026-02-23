//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_INDEX_BUFFER_H
#define VOXEL_GAME_INDEX_BUFFER_H

#include <span>

#include "glbinding/gl/bitfield.h"
#include "glbinding/gl/functions.h"
#include "glbinding/gl/types.h"

namespace gfx {
    namespace detail {
        [[nodiscard]] gl::GLuint make_gl_index_buffer();
    }

    template<typename T>
    concept Index = requires(T a)
    {
        std::is_integral_v<T> &&
        std::is_unsigned_v<T> &&
        (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4);
    };

    class IndexBuffer {
    public:
        template<Index I>
        [[nodiscard]] static IndexBuffer make_fixed(std::span<const I> data);

        template<Index I, std::size_t N>
        [[nodiscard]] static IndexBuffer make_fixed(const std::array<I, N>& arr) {
            return make_fixed<I>(std::span{arr});
        }

        [[nodiscard]] constexpr gl::GLuint handle() const noexcept {
            return index_buffer;
        }

        static void bind(const IndexBuffer& ibo);

        ~IndexBuffer();
        IndexBuffer(IndexBuffer&&);
        IndexBuffer& operator=(IndexBuffer&&);

    private:
        constexpr explicit IndexBuffer(const gl::GLuint index_buffer) : index_buffer(index_buffer) {}
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;
        gl::GLuint index_buffer{};
    };

    template<Index I>
    IndexBuffer IndexBuffer::make_fixed(const std::span<const I> data) {
        IndexBuffer ibo{detail::make_gl_index_buffer()};
        gl::glNamedBufferStorage(ibo.index_buffer,
            data.size_bytes(),
            data.data(),
            gl::BufferStorageMask::GL_DYNAMIC_STORAGE_BIT);
        return ibo;
    }

}

#endif //VOXEL_GAME_INDEX_BUFFER_H