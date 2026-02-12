//
// Created by Jamie on 11/02/2026.
//
#include "shader.h"

#include <fstream>
#include <optional>

#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"

#include "glm/mat4x4.hpp"

static gl::GLuint make_shader (const gl::GLenum type, const std::string_view source) {
    const char* source_ptr = source.data();
    const gl::GLuint shader = gl::glCreateShader(type);
    gl::glShaderSource(shader, 1, &source_ptr, nullptr);
    gl::glCompileShader(shader);
    return shader;
};

static gl::GLuint make_program(const gl::GLuint vertex_shader, const gl::GLuint fragment_shader) {
    const gl::GLuint program = gl::glCreateProgram();
    gl::glAttachShader(program, vertex_shader);
    gl::glAttachShader(program, fragment_shader);
    gl::glLinkProgram(program);
    return program;
}

static std::optional<gfx::LinkerError> make_link_error(
    const std::string_view vertex_source,
    const std::string_view fragment_source,
    const gl::GLuint program) {

    gl::GLint link_status{};
    gl::glGetProgramiv(program, gl::GL_LINK_STATUS, &link_status);

    // ok
    if (link_status == static_cast<gl::GLuint>(gl::GL_TRUE)) {
        return std::nullopt;
    }

    gl::GLsizei log_length{};
    gl::glGetProgramiv(program, gl::GL_INFO_LOG_LENGTH, &log_length);

    std::string log;
    log.resize(log_length);
    gl::glGetProgramInfoLog(program, log_length, nullptr, log.data());

    return gfx::LinkerError(vertex_source, fragment_source, log);
}

std::expected<gfx::Shader, gfx::LinkerError> gfx::Shader::from_files(
    const std::string_view vertex_filepath,
    const std::string_view fragment_filepath
) {
    const auto read_file = [](const std::string_view filepath) {
        std::ifstream fs (std::string(filepath.begin(), filepath.end()));
        std::string contents;
        std::getline(fs, contents, '\0');
        return contents;
    };

    const auto vertex_source = read_file(vertex_filepath);
    const auto fragment_source = read_file(fragment_filepath);
    const auto vertex_shader = make_shader(gl::GL_VERTEX_SHADER, vertex_source);
    const auto fragment_shader = make_shader(gl::GL_FRAGMENT_SHADER, fragment_source);

    Shader shader (make_program(vertex_shader, fragment_shader));
    const auto error = make_link_error(vertex_source, fragment_source, shader.program);

    // cleanup
    gl::glDeleteShader(vertex_shader);
    gl::glDeleteShader(fragment_shader);

    if (error) return std::unexpected(*error);

    return shader;
}

std::expected<gfx::Shader, gfx::LinkerError>  gfx::Shader::from_source(const std::string_view vertex_source, const std::string_view fragment_source) {
    const auto vertex_shader = make_shader(gl::GL_VERTEX_SHADER, vertex_source);
    const auto fragment_shader = make_shader(gl::GL_FRAGMENT_SHADER, fragment_source);
    Shader shader (make_program(vertex_shader, fragment_shader));
    const auto error = make_link_error(vertex_source, fragment_source, shader.program);
    // cleanup
    gl::glDeleteShader(vertex_shader);
    gl::glDeleteShader(fragment_shader);

    if (error) return std::unexpected(*error);

    return shader;
}

void gfx::Shader::bind(const Shader &shader) {
    gl::glUseProgram(shader.program);
}

void gfx::Shader::uniform_matrix(std::string_view name, const glm::mat4 &matrix) const {
    const gl::GLint location = gl::glGetUniformLocation(program, name.data());
    if (location == -1) return;
    gl::glProgramUniformMatrix4fv(program, location, 1, gl::GL_FALSE, &matrix[0][0]);
}

gfx::Shader::~Shader() {
    gl::glDeleteProgram(program);
}

gfx::Shader::Shader(Shader && temp) {
    program = temp.program;
    temp.program = 0;
}

gfx::Shader& gfx::Shader::operator=(Shader&& temp) {
    gl::glDeleteProgram(program);
    program = temp.program;
    temp.program = 0;
    return *this;
}

std::ostream & operator<<(std::ostream & os, const gfx::LinkerError& err) {
    os << "Begin Linker Error\n";
    os << "Log: " << err.log << '\n';
    os << "Vertex Source: " << err.vertex_source << '\n';
    os << "Fragment Source: " << err.fragment_source << '\n';
    os << "End Linker Error\n";
    return os;
}
