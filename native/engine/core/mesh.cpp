#include "engine/core/mesh.h"

#include <array>
#include <utility>

namespace engine {

Mesh::~Mesh() {
    Destroy();
}

Mesh::Mesh(Mesh&& other) noexcept {
    *this = std::move(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    Destroy();

    vao_ = std::exchange(other.vao_, 0U);
    vbo_ = std::exchange(other.vbo_, 0U);
    ibo_ = std::exchange(other.ibo_, 0U);
    indexCount_ = std::exchange(other.indexCount_, 0);
    vertexCount_ = std::exchange(other.vertexCount_, 0);

    return *this;
}

bool Mesh::Initialize(const std::vector<float>& positions,
                      const std::vector<float>& normals,
                      const std::vector<uint32_t>& indices) {
    Destroy();

    if (positions.empty()) {
        return false;
    }

    const size_t vertexCount = positions.size() / 3;
    const bool hasNormals = normals.size() == positions.size();
    std::vector<float> interleaved;
    interleaved.reserve(vertexCount * 6);

    for (size_t i = 0; i < vertexCount; ++i) {
        interleaved.push_back(positions[i * 3 + 0]);
        interleaved.push_back(positions[i * 3 + 1]);
        interleaved.push_back(positions[i * 3 + 2]);

        if (hasNormals) {
            interleaved.push_back(normals[i * 3 + 0]);
            interleaved.push_back(normals[i * 3 + 1]);
            interleaved.push_back(normals[i * 3 + 2]);
        } else {
            interleaved.push_back(0.0f);
            interleaved.push_back(1.0f);
            interleaved.push_back(0.0f);
        }
    }

    Upload(interleaved, indices);
    return IsValid();
}

void Mesh::InitializePlaceholder(float scale) {
    Destroy();

    const float s = scale;
    const std::array<float, 24> positions = {
        -s, -s, -s,
         s, -s, -s,
         s,  s, -s,
        -s,  s, -s,
        -s, -s,  s,
         s, -s,  s,
         s,  s,  s,
        -s,  s,  s
    };

    const std::array<float, 24> normals = {
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };

    const std::array<uint32_t, 36> indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        0, 1, 5, 0, 5, 4,
        2, 3, 7, 2, 7, 6,
        0, 3, 7, 0, 7, 4,
        1, 2, 6, 1, 6, 5
    };

    std::vector<float> interleaved;
    interleaved.reserve(positions.size() + normals.size());
    for (size_t i = 0; i < positions.size() / 3; ++i) {
        interleaved.push_back(positions[i * 3 + 0]);
        interleaved.push_back(positions[i * 3 + 1]);
        interleaved.push_back(positions[i * 3 + 2]);
        interleaved.push_back(normals[i * 3 + 0]);
        interleaved.push_back(normals[i * 3 + 1]);
        interleaved.push_back(normals[i * 3 + 2]);
    }

    std::vector<uint32_t> indexBuffer(indices.begin(), indices.end());
    Upload(interleaved, indexBuffer);
}

void Mesh::Destroy() {
    if (ibo_ != 0) {
        glDeleteBuffers(1, &ibo_);
        ibo_ = 0;
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    indexCount_ = 0;
    vertexCount_ = 0;
}

void Mesh::Draw() const {
    if (!IsValid()) {
        return;
    }

    glBindVertexArray(vao_);
    if (indexCount_ > 0) {
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    }
    glBindVertexArray(0);
}

void Mesh::Upload(const std::vector<float>& interleaved,
                  const std::vector<uint32_t>& indices) {
    if (interleaved.empty()) {
        return;
    }

    vertexCount_ = static_cast<GLsizei>(interleaved.size() / 6);
    indexCount_ = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(interleaved.size() * sizeof(float)),
                 interleaved.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    if (!indices.empty()) {
        glGenBuffers(1, &ibo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
                     indices.data(),
                     GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

}  // namespace engine
