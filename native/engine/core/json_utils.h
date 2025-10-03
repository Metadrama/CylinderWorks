#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

class JsonValue {
public:
    enum class Type {
        kNull,
        kBool,
        kNumber,
        kString,
        kArray,
        kObject
    };

    JsonValue() = default;

    bool IsNull() const { return type_ == Type::kNull; }
    bool IsBool() const { return type_ == Type::kBool; }
    bool IsNumber() const { return type_ == Type::kNumber; }
    bool IsString() const { return type_ == Type::kString; }
    bool IsArray() const { return type_ == Type::kArray; }
    bool IsObject() const { return type_ == Type::kObject; }

    bool AsBool(bool fallback) const;
    double AsNumber(double fallback) const;
    std::string AsString(const std::string& fallback) const;

    const std::vector<JsonValue>& AsArray() const;
    const std::unordered_map<std::string, JsonValue>& AsObject() const;

    std::vector<JsonValue>& MutableArray();
    std::unordered_map<std::string, JsonValue>& MutableObject();

    bool Contains(const std::string& key) const;
    const JsonValue& operator[](const std::string& key) const;
    const JsonValue& operator[](size_t index) const;

    size_t Size() const;

    void SetNull();
    void SetBool(bool value);
    void SetNumber(double value);
    void SetString(const std::string& value);
    void SetArray();
    void SetObject();

private:
    Type type_{Type::kNull};
    bool boolValue_{false};
    double numberValue_{0.0};
    std::string stringValue_;
    std::vector<JsonValue> arrayValue_;
    std::unordered_map<std::string, JsonValue> objectValue_;
};

bool ParseJson(const std::string& text, JsonValue* outValue);

}  // namespace engine
