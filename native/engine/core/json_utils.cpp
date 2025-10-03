#include "engine/core/json_utils.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace engine {
namespace {

class JsonParser {
public:
    JsonParser(const char* begin, const char* end) : current_(begin), end_(end) {}

    bool Parse(JsonValue* outValue) {
        SkipWhitespace();
        if (!ParseValue(outValue)) {
            return false;
        }
        SkipWhitespace();
        return current_ == end_;
    }

private:
    bool ParseValue(JsonValue* outValue) {
        if (current_ == end_) {
            return false;
        }
        switch (*current_) {
            case 'n':
                return ParseLiteral("null", [&]() { outValue->SetNull(); });
            case 't':
                return ParseLiteral("true", [&]() { outValue->SetBool(true); });
            case 'f':
                return ParseLiteral("false", [&]() { outValue->SetBool(false); });
            case '"':
                return ParseString(outValue);
            case '[':
                return ParseArray(outValue);
            case '{':
                return ParseObject(outValue);
            default:
                return ParseNumber(outValue);
        }
    }

    template <typename Fn>
    bool ParseLiteral(const char* literal, Fn fn) {
        const size_t length = std::strlen(literal);
        if (static_cast<size_t>(end_ - current_) < length) {
            return false;
        }
        if (std::memcmp(current_, literal, length) != 0) {
            return false;
        }
        current_ += length;
        fn();
        return true;
    }

    bool ParseString(JsonValue* outValue) {
        std::string buffer;
        if (!ParseString(&buffer)) {
            return false;
        }
        outValue->SetString(buffer);
        return true;
    }

    bool ParseString(std::string* out) {
        if (current_ == end_ || *current_ != '"') {
            return false;
        }
        ++current_;
        std::string result;
        while (current_ != end_) {
            const char ch = *current_;
            if (ch == '"') {
                ++current_;
                *out = result;
                return true;
            }
            if (ch == '\\') {
                ++current_;
                if (current_ == end_) {
                    return false;
                }
                char escaped = *current_;
                switch (escaped) {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    default:
                        return false;
                }
            } else {
                result.push_back(ch);
            }
            ++current_;
        }
        return false;
    }

    bool ParseArray(JsonValue* outValue) {
        if (current_ == end_ || *current_ != '[') {
            return false;
        }
        ++current_;
        outValue->SetArray();
        auto& array = outValue->MutableArray();

        SkipWhitespace();
        if (current_ != end_ && *current_ == ']') {
            ++current_;
            return true;
        }

        while (current_ != end_) {
            JsonValue element;
            if (!ParseValue(&element)) {
                return false;
            }
            array.push_back(std::move(element));
            SkipWhitespace();
            if (current_ == end_) {
                return false;
            }
            if (*current_ == ',') {
                ++current_;
                SkipWhitespace();
                continue;
            }
            if (*current_ == ']') {
                ++current_;
                return true;
            }
            return false;
        }
        return false;
    }

    bool ParseObject(JsonValue* outValue) {
        if (current_ == end_ || *current_ != '{') {
            return false;
        }
        ++current_;
        outValue->SetObject();
        auto& object = outValue->MutableObject();

        SkipWhitespace();
        if (current_ != end_ && *current_ == '}') {
            ++current_;
            return true;
        }

        while (current_ != end_) {
            SkipWhitespace();
            std::string key;
            if (!ParseString(&key)) {
                return false;
            }
            SkipWhitespace();
            if (current_ == end_ || *current_ != ':') {
                return false;
            }
            ++current_;
            SkipWhitespace();
            JsonValue value;
            if (!ParseValue(&value)) {
                return false;
            }
            object[key] = std::move(value);
            SkipWhitespace();
            if (current_ == end_) {
                return false;
            }
            if (*current_ == ',') {
                ++current_;
                SkipWhitespace();
                continue;
            }
            if (*current_ == '}') {
                ++current_;
                return true;
            }
            return false;
        }
        return false;
    }

    bool ParseNumber(JsonValue* outValue) {
        const char* start = current_;
        if (current_ != end_ && (*current_ == '-' || *current_ == '+')) {
            ++current_;
        }
        bool hasDigits = false;
        while (current_ != end_ && std::isdigit(static_cast<unsigned char>(*current_))) {
            ++current_;
            hasDigits = true;
        }
        if (current_ != end_ && *current_ == '.') {
            ++current_;
            while (current_ != end_ && std::isdigit(static_cast<unsigned char>(*current_))) {
                ++current_;
                hasDigits = true;
            }
        }
        if (!hasDigits) {
            return false;
        }
        if (current_ != end_ && (*current_ == 'e' || *current_ == 'E')) {
            ++current_;
            if (current_ != end_ && (*current_ == '+' || *current_ == '-')) {
                ++current_;
            }
            bool expDigits = false;
            while (current_ != end_ && std::isdigit(static_cast<unsigned char>(*current_))) {
                ++current_;
                expDigits = true;
            }
            if (!expDigits) {
                return false;
            }
        }

        const std::string token(start, current_);
        char* endPtr = nullptr;
        const double value = std::strtod(token.c_str(), &endPtr);
        if (endPtr != token.c_str() + token.size()) {
            return false;
        }
        outValue->SetNumber(value);
        return true;
    }

    void SkipWhitespace() {
        while (current_ != end_ && std::isspace(static_cast<unsigned char>(*current_))) {
            ++current_;
        }
    }

    const char* current_{nullptr};
    const char* end_{nullptr};
};

const JsonValue kNullValue;

}  // namespace

bool JsonValue::AsBool(bool fallback) const {
    return IsBool() ? boolValue_ : fallback;
}

double JsonValue::AsNumber(double fallback) const {
    return IsNumber() ? numberValue_ : fallback;
}

std::string JsonValue::AsString(const std::string& fallback) const {
    return IsString() ? stringValue_ : fallback;
}

const std::vector<JsonValue>& JsonValue::AsArray() const {
    return arrayValue_;
}

const std::unordered_map<std::string, JsonValue>& JsonValue::AsObject() const {
    return objectValue_;
}

std::vector<JsonValue>& JsonValue::MutableArray() {
    return arrayValue_;
}

std::unordered_map<std::string, JsonValue>& JsonValue::MutableObject() {
    return objectValue_;
}

bool JsonValue::Contains(const std::string& key) const {
    if (!IsObject()) {
        return false;
    }
    return objectValue_.find(key) != objectValue_.end();
}

const JsonValue& JsonValue::operator[](const std::string& key) const {
    if (!IsObject()) {
        return kNullValue;
    }
    auto it = objectValue_.find(key);
    if (it == objectValue_.end()) {
        return kNullValue;
    }
    return it->second;
}

const JsonValue& JsonValue::operator[](size_t index) const {
    if (!IsArray() || index >= arrayValue_.size()) {
        return kNullValue;
    }
    return arrayValue_[index];
}

size_t JsonValue::Size() const {
    if (IsArray()) {
        return arrayValue_.size();
    }
    if (IsObject()) {
        return objectValue_.size();
    }
    return 0;
}

void JsonValue::SetNull() {
    type_ = Type::kNull;
    stringValue_.clear();
    arrayValue_.clear();
    objectValue_.clear();
}

void JsonValue::SetBool(bool value) {
    type_ = Type::kBool;
    boolValue_ = value;
}

void JsonValue::SetNumber(double value) {
    type_ = Type::kNumber;
    numberValue_ = value;
}

void JsonValue::SetString(const std::string& value) {
    type_ = Type::kString;
    stringValue_ = value;
}

void JsonValue::SetArray() {
    type_ = Type::kArray;
    arrayValue_.clear();
}

void JsonValue::SetObject() {
    type_ = Type::kObject;
    objectValue_.clear();
}

bool ParseJson(const std::string& text, JsonValue* outValue) {
    if (!outValue) {
        return false;
    }
    JsonParser parser(text.data(), text.data() + text.size());
    return parser.Parse(outValue);
}

}  // namespace engine
