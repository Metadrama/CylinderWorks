#define CGLTF_IMPLEMENTATION
#include "engine/core/cgltf.h"
#include "engine/core/gltf_model.h"
#include <android/log.h>

namespace engine {

namespace {
constexpr const char* kTag = "GltfModel";

const char* kDefaultVertexShader = R"(
#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4 uViewProj;
uniform mat4 uModel;
out vec3 vNormal;
out vec3 vWorldPos;
void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = mat3(uModel) * aNormal;
    gl_Position = uViewProj * worldPos;
}
)";

const char* kDefaultFragmentShader = R"(
#version 300 es
precision mediump float;
in vec3 vNormal;
in vec3 vWorldPos;
uniform vec3 uCameraPos;
out vec4 fragColor;
void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(1.0, 1.0, 1.0)); // Fixed light direction
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    vec3 ambient = vec3(0.1, 0.1, 0.12);
    vec3 diffuse = vec3(0.4, 0.45, 0.5) * diff;
    vec3 specular = vec3(0.3, 0.3, 0.3) * spec;

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
)";

} // namespace

ShaderProgram GltfModel::shader_;
int GltfModel::shaderRefCount_ = 0;

GltfModel::GltfModel() {
    InitializeShader();
}

GltfModel::~GltfModel() {
    Destroy();
    ReleaseShader();
}

void GltfModel::InitializeShader() {
    if (shaderRefCount_ == 0) {
        if (!shader_.Compile(kDefaultVertexShader, kDefaultFragmentShader)) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "Failed to compile GLTF shader");
        }
    }
    shaderRefCount_++;
}

void GltfModel::ReleaseShader() {
    shaderRefCount_--;
    if (shaderRefCount_ == 0) {
        shader_.Destroy();
    }
}

bool GltfModel::Load(const void* data, size_t size) {
    cgltf_options options = {};
    cgltf_data* gltf_data = nullptr;
    cgltf_result result = cgltf_parse(&options, data, size, &gltf_data);

    if (result != cgltf_result_success) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Failed to parse GLTF data: %d", (int)result);
        return false;
    }

    result = cgltf_load_buffers(&options, gltf_data, nullptr);
    if (result != cgltf_result_success) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Failed to load GLTF buffers: %d", (int)result);
        cgltf_free(gltf_data);
        return false;
    }

    for (cgltf_size i = 0; i < gltf_data->meshes_count; ++i) {
        const cgltf_mesh& mesh = gltf_data->meshes[i];
        for (cgltf_size j = 0; j < mesh.primitives_count; ++j) {
            const cgltf_primitive& prim = mesh.primitives[j];

            GltfMesh glMesh;
            glGenVertexArrays(1, &glMesh.vao);
            glBindVertexArray(glMesh.vao);

            // Position
            cgltf_accessor* posAttr = nullptr;
            for (cgltf_size k = 0; k < prim.attributes_count; ++k) {
                if (prim.attributes[k].type == cgltf_attribute_type_position) {
                    posAttr = prim.attributes[k].data;
                    break;
                }
            }

            if (posAttr) {
                GLuint vbo;
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                
                size_t posSize = posAttr->count * sizeof(float) * 3;
                std::vector<float> posData(posAttr->count * 3);
                for (cgltf_size k = 0; k < posAttr->count; ++k) {
                    cgltf_accessor_read_float(posAttr, k, &posData[k * 3], 3);
                }
                glBufferData(GL_ARRAY_BUFFER, posSize, posData.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                glMesh.vbos.push_back(vbo);
            }

            // Normals
            cgltf_accessor* normAttr = nullptr;
            for (cgltf_size k = 0; k < prim.attributes_count; ++k) {
                if (prim.attributes[k].type == cgltf_attribute_type_normal) {
                    normAttr = prim.attributes[k].data;
                    break;
                }
            }

            if (normAttr) {
                GLuint vbo;
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                size_t normSize = normAttr->count * sizeof(float) * 3;
                std::vector<float> normData(normAttr->count * 3);
                for (cgltf_size k = 0; k < normAttr->count; ++k) {
                    cgltf_accessor_read_float(normAttr, k, &normData[k * 3], 3);
                }
                glBufferData(GL_ARRAY_BUFFER, normSize, normData.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                glMesh.vbos.push_back(vbo);
            }

            // Indices
            if (prim.indices) {
                glGenBuffers(1, &glMesh.ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ebo);
                glMesh.count = (GLsizei)prim.indices->count;
                
                size_t indexSize = prim.indices->count * sizeof(uint32_t);
                std::vector<uint32_t> indexData(prim.indices->count);
                for (cgltf_size k = 0; k < prim.indices->count; ++k) {
                    indexData[k] = (uint32_t)cgltf_accessor_read_index(prim.indices, k);
                }
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexData.data(), GL_STATIC_DRAW);
            } else {
                glMesh.count = (GLsizei)posAttr->count;
            }

            meshes_.push_back(glMesh);
            glBindVertexArray(0);
        }
    }

    cgltf_free(gltf_data);
    isLoaded_ = true;
    return true;
}

void GltfModel::Destroy() {
    for (auto& mesh : meshes_) {
        glDeleteVertexArrays(1, &mesh.vao);
        for (auto vbo : mesh.vbos) {
            glDeleteBuffers(1, &vbo);
        }
        if (mesh.ebo) {
            glDeleteBuffers(1, &mesh.ebo);
        }
    }
    meshes_.clear();
    isLoaded_ = false;
}

void GltfModel::Draw(const Mat4& viewProj, const Mat4& model, const Vec3& cameraPos) {
    if (!isLoaded_) return;

    glUseProgram(shader_.Id());
    
    GLint uViewProj = glGetUniformLocation(shader_.Id(), "uViewProj");
    GLint uModel = glGetUniformLocation(shader_.Id(), "uModel");
    GLint uCameraPos = glGetUniformLocation(shader_.Id(), "uCameraPos");

    glUniformMatrix4fv(uViewProj, 1, GL_FALSE, viewProj.Ptr());
    glUniformMatrix4fv(uModel, 1, GL_FALSE, model.Ptr());
    glUniform3f(uCameraPos, cameraPos.x, cameraPos.y, cameraPos.z);

    for (const auto& mesh : meshes_) {
        glBindVertexArray(mesh.vao);
        if (mesh.ebo) {
            glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, mesh.count);
        }
    }
    glBindVertexArray(0);
}

} // namespace engine
