#pragma once

#include <array>
#include <cmath>

namespace engine {

struct Vec2 {
    float x{0.0f};
    float y{0.0f};
};

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};

    Vec3() = default;
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct Vec4 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float w{0.0f};

    Vec4() = default;
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs) {
    return Vec3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs) {
    return Vec3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

inline Vec3 operator*(const Vec3& lhs, float scalar) {
    return Vec3{lhs.x * scalar, lhs.y * scalar, lhs.z * scalar};
}

inline Vec3 operator/(const Vec3& lhs, float scalar) {
    const float inv = 1.0f / scalar;
    return Vec3{lhs.x * inv, lhs.y * inv, lhs.z * inv};
}

inline Vec3& operator+=(Vec3& lhs, const Vec3& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

inline Vec3& operator-=(Vec3& lhs, const Vec3& rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs;
}

inline float Dot(const Vec3& lhs, const Vec3& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline Vec3 Cross(const Vec3& lhs, const Vec3& rhs) {
    return Vec3{
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x
    };
}

inline float Length(const Vec3& v) {
    return std::sqrt(Dot(v, v));
}

inline Vec3 Normalize(const Vec3& v) {
    const float len = Length(v);
    if (len <= 0.0f) {
        return Vec3{0.0f, 0.0f, 0.0f};
    }
    return v / len;
}

struct Mat4 {
    std::array<float, 16> data{0.0f};

    float* Ptr() { return data.data(); }
    const float* Ptr() const { return data.data(); }

    static Mat4 Identity() {
        Mat4 m;
        m.data = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return m;
    }
};

inline Mat4 Multiply(const Mat4& a, const Mat4& b) {
    Mat4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a.data[k * 4 + row] * b.data[col * 4 + k];
            }
            result.data[col * 4 + row] = sum;
        }
    }
    return result;
}

inline Mat4 Perspective(float fovyRadians, float aspect, float zNear, float zFar) {
    const float tanHalfFovy = std::tan(fovyRadians * 0.5f);
    Mat4 result{};
    result.data = {
        1.0f / (aspect * tanHalfFovy), 0.0f,                        0.0f,                               0.0f,
        0.0f,                               1.0f / tanHalfFovy,     0.0f,                               0.0f,
        0.0f,                               0.0f,                   -(zFar + zNear) / (zFar - zNear),   -1.0f,
        0.0f,                               0.0f,                   -(2.0f * zFar * zNear) / (zFar - zNear), 0.0f
    };
    return result;
}

inline Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = Normalize(center - eye);
    Vec3 s = Normalize(Cross(f, up));
    Vec3 u = Cross(s, f);

    Mat4 result = Mat4::Identity();
    result.data[0] = s.x;
    result.data[4] = s.y;
    result.data[8] = s.z;

    result.data[1] = u.x;
    result.data[5] = u.y;
    result.data[9] = u.z;

    result.data[2] = -f.x;
    result.data[6] = -f.y;
    result.data[10] = -f.z;

    result.data[12] = -Dot(s, eye);
    result.data[13] = -Dot(u, eye);
    result.data[14] = Dot(f, eye);

    return result;
}

inline float Clamp(float value, float minValue, float maxValue) {
    return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}

inline float Radians(float degrees) {
    return degrees * 0.01745329251994329577f;
}

inline Mat4 Translation(const Vec3& t) {
    Mat4 m = Mat4::Identity();
    m.data[12] = t.x;
    m.data[13] = t.y;
    m.data[14] = t.z;
    return m;
}

inline Mat4 Scale(const Vec3& s) {
    Mat4 m{};
    m.data = {
        s.x, 0.0f, 0.0f, 0.0f,
        0.0f, s.y, 0.0f, 0.0f,
        0.0f, 0.0f, s.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return m;
}

inline Mat4 RotationXYZ(const Vec3& degrees) {
    const float rx = Radians(degrees.x);
    const float ry = Radians(degrees.y);
    const float rz = Radians(degrees.z);

    const float cx = std::cos(rx);
    const float sx = std::sin(rx);
    const float cy = std::cos(ry);
    const float sy = std::sin(ry);
    const float cz = std::cos(rz);
    const float sz = std::sin(rz);

    Mat4 result = Mat4::Identity();

    result.data[0] = cy * cz;
    result.data[4] = cy * sz;
    result.data[8] = -sy;

    result.data[1] = sx * sy * cz - cx * sz;
    result.data[5] = sx * sy * sz + cx * cz;
    result.data[9] = sx * cy;

    result.data[2] = cx * sy * cz + sx * sz;
    result.data[6] = cx * sy * sz - sx * cz;
    result.data[10] = cx * cy;

    return result;
}

inline Mat4 ComposeTransform(const Vec3& translation, const Vec3& rotationDegrees) {
    const Mat4 rot = RotationXYZ(rotationDegrees);
    const Mat4 trans = Translation(translation);
    return Multiply(trans, rot);
}

inline Mat4 InvertRigidTransform(const Mat4& transform) {
    Mat4 result = Mat4::Identity();

    // Extract rotation matrix (upper-left 3x3) and transpose it.
    result.data[0] = transform.data[0];
    result.data[1] = transform.data[4];
    result.data[2] = transform.data[8];

    result.data[4] = transform.data[1];
    result.data[5] = transform.data[5];
    result.data[6] = transform.data[9];

    result.data[8] = transform.data[2];
    result.data[9] = transform.data[6];
    result.data[10] = transform.data[10];

    const Vec3 translation{transform.data[12], transform.data[13], transform.data[14]};
    const Vec3 rotated{
        result.data[0] * translation.x + result.data[4] * translation.y + result.data[8] * translation.z,
        result.data[1] * translation.x + result.data[5] * translation.y + result.data[9] * translation.z,
        result.data[2] * translation.x + result.data[6] * translation.y + result.data[10] * translation.z
    };

    result.data[12] = -rotated.x;
    result.data[13] = -rotated.y;
    result.data[14] = -rotated.z;

    return result;
}

inline Mat4 CombineAttachmentTransforms(const Mat4& parentAttachment, const Mat4& selfAttachment) {
    return Multiply(parentAttachment, InvertRigidTransform(selfAttachment));
}

}  // namespace engine

