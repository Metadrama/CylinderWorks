#pragma once

#include <vector>
#include <string>
#include <GLES3/gl3.h>
#include "engine/core/math_types.h"
#include "engine/core/shader_program.h"

namespace engine {

struct GltfMesh {
    GLuint vao{0};
    std::vector<GLuint> vbos;
    GLuint ebo{0};
    GLsizei count{0};
};

class GltfModel {
public:
    GltfModel();
    ~GltfModel();

    bool Load(const void* data, size_t size);
    void Destroy();
    void Draw(const Mat4& viewProj, const Mat4& model, const Vec3& cameraPos);

private:
    std::vector<GltfMesh> meshes_;
    bool isLoaded_{false};

    static const char* kVertexShader;
    static const char* kFragmentShader;
    static ShaderProgram shader_;
    static int shaderRefCount_;

    void InitializeShader();
    void ReleaseShader();
};

}  // namespace engine
