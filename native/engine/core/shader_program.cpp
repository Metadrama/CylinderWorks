#include "shader_program.h"

#include <android/log.h>

namespace engine {

namespace {
constexpr const char* kTag = "EngineRenderer";
}

ShaderProgram::~ShaderProgram() {
    Destroy();
}

bool ShaderProgram::Compile(const char* vertexSrc, const char* fragmentSrc) {
    Destroy();

    const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    if (!vertexShader) {
        return false;
    }

    const GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vertexShader);
    glAttachShader(program_, fragmentShader);
    glLinkProgram(program_);

    GLint status = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logLength);
        std::string log;
        log.resize(static_cast<std::size_t>(logLength));
        glGetProgramInfoLog(program_, logLength, nullptr, log.data());
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Program link failed: %s", log.c_str());

        glDeleteProgram(program_);
        program_ = 0;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    glDetachShader(program_, vertexShader);
    glDetachShader(program_, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void ShaderProgram::Destroy() {
    if (program_ != 0) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

GLuint ShaderProgram::CompileShader(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string log;
        log.resize(static_cast<std::size_t>(logLength));
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Shader compile failed: %s", log.c_str());
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

}  // namespace engine
