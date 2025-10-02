#pragma once

#include <GLES3/gl3.h>
#include <string>

namespace engine {

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();

    bool Compile(const char* vertexSrc, const char* fragmentSrc);
    void Destroy();

    GLuint Id() const { return program_; }

private:
    GLuint CompileShader(GLenum type, const char* source);

    GLuint program_{0};
};

}  // namespace engine
