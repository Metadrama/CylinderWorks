#pragma once

#include <GLES3/gl3.h>

namespace engine {

class GridPlane {
public:
    GridPlane() = default;
    ~GridPlane();

    void Initialize();
    void Destroy();
    void Draw() const;

private:
    GLuint vao_{0};
    GLuint vbo_{0};
};

}  // namespace engine
