//
// Created by Jamie on 11/02/2026.
//

#ifndef VOXEL_GAME_SHADER_H
#define VOXEL_GAME_SHADER_H
#include <string_view>
#include <optional>

#include "glbinding/gl/types.h"

namespace gfx {
    class Shader {
    public:
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

        [[nodiscard]] static Shader from_files(
            std::string_view vertex_filepath,
            std::string_view fragment_filepath
        );

        [[nodiscard]] static Shader from_source(
            std::string_view vertex_source,
            std::string_view fragment_source
        );

        [[nodiscard]] constexpr auto linker_error() const noexcept {
            return error;
        }

        static void bind(const Shader &shader);
        ~Shader();
        Shader(Shader &&);
        Shader &operator =(Shader &&);
    private:
        constexpr explicit Shader(const gl::GLuint program) : program (program) {}

        Shader(const Shader &) = delete;
        Shader &operator =(const Shader &) = delete;

        gl::GLuint program{};
        std::optional<LinkerError> error{};
    };
}


std::ostream &operator<<(std::ostream&, const gfx::Shader::LinkerError&);

#endif //VOXEL_GAME_SHADER_H
