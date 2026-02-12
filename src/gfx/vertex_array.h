//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_VERTEX_ARRAY_H
#define VOXEL_GAME_VERTEX_ARRAY_H
#include <glbinding/gl10/types.h>

#include "glm/vec3.hpp"

namespace gfx {

    struct VertexTesting {
        glm::vec3 xyz{};
        glm::vec3 rgb{};
    };

    class VertexBuffer;
    class IndexBuffer;

    class VertexArray {
    public:
        [[nodiscard]] static VertexArray make_testing(const VertexBuffer& vertex_buffer, const IndexBuffer& index_buffer);
        static void bind(const VertexArray& vao);
        ~VertexArray();
        VertexArray(VertexArray&&);
        VertexArray& operator=(VertexArray&&);
    private:
        constexpr VertexArray(const gl::GLuint vertex_array) :
            vertex_array{vertex_array} {}

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator = (const VertexArray&) = delete;
        gl::GLuint vertex_array{};
    };
}

#endif //VOXEL_GAME_VERTEX_ARRAY_H