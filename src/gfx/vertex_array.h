//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_VERTEX_ARRAY_H
#define VOXEL_GAME_VERTEX_ARRAY_H
#include <glbinding/gl10/types.h>

namespace gfx {

    class VertexBuffer;

    class VertexArray {
    public:
        [[nodiscard]] static VertexArray make_testing(const VertexBuffer& vertex_buffer);
        void bind() const;

        ~VertexArray();
        VertexArray(VertexArray&&);
        VertexArray& operator=(VertexArray&&);
    private:
        VertexArray() = default;
        VertexArray(const VertexArray&) = delete;
        VertexArray& operator = (const VertexArray&) = delete;
        gl::GLuint vertex_array{};
    };
}

#endif //VOXEL_GAME_VERTEX_ARRAY_H