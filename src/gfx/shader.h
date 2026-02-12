//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_SHADER_H
#define VOXEL_GAME_SHADER_H
#include <string_view>
#include <expected>

#include "glbinding/gl/types.h"
#include "glm/fwd.hpp"

namespace gfx {
    struct LinkerError {
        std::string vertex_source;
        std::string fragment_source;
        std::string log;

        constexpr explicit LinkerError(
            const std::string_view vertex_source,
            const std::string_view fragment_source,
            const std::string_view log) :
                vertex_source(vertex_source),
                fragment_source(fragment_source),
                log(log) {}
    };

    class Shader {
    public:
        [[nodiscard]] static std::expected<Shader, LinkerError> from_files(
            std::string_view vertex_filepath,
            std::string_view fragment_filepath
        );

        [[nodiscard]] static std::expected<Shader, LinkerError> from_source(
            std::string_view vertex_source,
            std::string_view fragment_source
        );

        static void bind(const Shader &shader);
        void uniform_matrix(std::string_view name, const glm::mat4& matrix) const;

        ~Shader();
        Shader(Shader &&);
        Shader &operator =(Shader &&);
    private:
        constexpr explicit Shader(const gl::GLuint program) : program (program) {}
        Shader(const Shader &) = delete;
        Shader &operator =(const Shader &) = delete;
        gl::GLuint program{};
    };
}


std::ostream &operator<<(std::ostream&, const gfx::LinkerError&);

#endif //VOXEL_GAME_SHADER_H
