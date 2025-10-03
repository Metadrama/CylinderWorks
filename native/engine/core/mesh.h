#pragma once

#include <GLES3/gl3.h>
#include <vector>

#include "engine/core/math_types.h"

namespace engine {

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    bool Initialize(const std::vector<float>& positions,
                    const std::vector<float>& normals,
                    const std::vector<uint32_t>& indices);
    void InitializePlaceholder(float scale);
    void Destroy();

    bool IsValid() const { return vao_ != 0; }
    void Draw() const;

private:
    void Upload(const std::vector<float>& interleaved,
                const std::vector<uint32_t>& indices);

    GLuint vao_{0};
    GLuint vbo_{0};
    GLuint ibo_{0};
    GLsizei indexCount_{0};
    GLsizei vertexCount_{0};
};

}  // namespace engine
