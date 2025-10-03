#include "engine/core/gltf_loader.h"

#include <android/asset_manager.h>
#include <android/log.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "engine/core/json_utils.h"

namespace engine {
namespace {
constexpr const char* kTag = "EngineRenderer";

struct GlbChunks {
    std::string json;
    std::vector<uint8_t> bin;
};

bool ReadAssetFile(AAssetManager* manager, const std::string& path, std::vector<uint8_t>* outData) {
    if (!manager || !outData) {
        return false;
    }
    AAsset* asset = AAssetManager_open(manager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        return false;
    }
    const off_t length = AAsset_getLength(asset);
    outData->resize(static_cast<size_t>(length));
    const int64_t read = AAsset_read(asset, outData->data(), length);
    AAsset_close(asset);
    return read == length;
}

bool ExtractGlbChunks(const std::vector<uint8_t>& data, GlbChunks* outChunks, std::string* outError) {
    if (data.size() < 20) {
        if (outError) {
            *outError = "GLB file too small";
        }
        return false;
    }

    uint32_t magic = 0;
    std::memcpy(&magic, data.data(), sizeof(uint32_t));
    if (magic != 0x46546C67) {  // 'glTF'
        if (outError) {
            *outError = "Invalid GLB magic";
        }
        return false;
    }

    uint32_t version = 0;
    std::memcpy(&version, data.data() + 4, sizeof(uint32_t));
    if (version != 2) {
        if (outError) {
            *outError = "Unsupported GLB version";
        }
        return false;
    }

    size_t offset = 12;  // Skip header.
    std::string jsonChunk;
    std::vector<uint8_t> binChunk;

    while (offset + 8 <= data.size()) {
    uint32_t chunkLength = 0;
    uint32_t chunkType = 0;
    std::memcpy(&chunkLength, data.data() + offset, sizeof(uint32_t));
    std::memcpy(&chunkType, data.data() + offset + 4, sizeof(uint32_t));
        offset += 8;
        if (offset + chunkLength > data.size()) {
            if (outError) {
                *outError = "GLB chunk length overflow";
            }
            return false;
        }

        if (chunkType == 0x4E4F534A) {  // 'JSON'
            jsonChunk.assign(reinterpret_cast<const char*>(data.data() + offset), chunkLength);
        } else if (chunkType == 0x004E4942) {  // 'BIN\0'
            binChunk.assign(data.begin() + static_cast<long>(offset),
                            data.begin() + static_cast<long>(offset + chunkLength));
        }
        offset += chunkLength;
    }

    if (jsonChunk.empty()) {
        if (outError) {
            *outError = "Missing JSON chunk";
        }
        return false;
    }

    if (binChunk.empty()) {
        if (outError) {
            *outError = "Missing BIN chunk";
        }
        return false;
    }

    outChunks->json = std::move(jsonChunk);
    outChunks->bin = std::move(binChunk);
    return true;
}

int ComponentTypeByteSize(int componentType) {
    switch (componentType) {
        case 5120:  // BYTE
        case 5121:  // UNSIGNED_BYTE
            return 1;
        case 5122:  // SHORT
        case 5123:  // UNSIGNED_SHORT
            return 2;
        case 5125:  // UNSIGNED_INT
        case 5126:  // FLOAT
            return 4;
        default:
            return 0;
    }
}

int TypeComponentCount(const std::string& type) {
    if (type == "SCALAR") return 1;
    if (type == "VEC2") return 2;
    if (type == "VEC3") return 3;
    if (type == "VEC4") return 4;
    if (type == "MAT4") return 16;
    return 0;
}

struct AccessorView {
    const uint8_t* data{nullptr};
    size_t count{0};
    size_t stride{0};
    int componentType{0};
    std::string type;
};

bool ResolveAccessor(const JsonValue& doc,
                     int accessorIndex,
                     const std::vector<uint8_t>& binData,
                     AccessorView* outView,
                     std::string* outError) {
    const JsonValue& accessors = doc["accessors"];
    if (!accessors.IsArray() || accessorIndex < 0 || accessorIndex >= static_cast<int>(accessors.Size())) {
        if (outError) {
            *outError = "Accessor index out of range";
        }
        return false;
    }

    const JsonValue& accessor = accessors[accessorIndex];
    if (!accessor.Contains("bufferView")) {
        if (outError) {
            *outError = "Accessor missing bufferView";
        }
        return false;
    }

    const int bufferViewIndex = static_cast<int>(accessor["bufferView"].AsNumber(-1));
    const JsonValue& bufferViews = doc["bufferViews"];
    if (!bufferViews.IsArray() || bufferViewIndex < 0 ||
        bufferViewIndex >= static_cast<int>(bufferViews.Size())) {
        if (outError) {
            *outError = "BufferView index out of range";
        }
        return false;
    }

    const JsonValue& bufferView = bufferViews[bufferViewIndex];
    const int bufferIndex = static_cast<int>(bufferView["buffer"].AsNumber(0.0));
    if (bufferIndex != 0) {
        if (outError) {
            *outError = "Only single-buffer GLB files are supported";
        }
        return false;
    }

    const size_t byteOffsetView = static_cast<size_t>(bufferView["byteOffset"].AsNumber(0.0));
    const size_t byteStride = static_cast<size_t>(bufferView["byteStride"].AsNumber(0.0));
    const size_t byteLength = static_cast<size_t>(bufferView["byteLength"].AsNumber(0.0));

    const size_t accessorOffset = static_cast<size_t>(accessor["byteOffset"].AsNumber(0.0));
    const size_t count = static_cast<size_t>(accessor["count"].AsNumber(0.0));
    const std::string type = accessor["type"].AsString("");
    const int componentType = static_cast<int>(accessor["componentType"].AsNumber(0.0));

    const int componentSize = ComponentTypeByteSize(componentType);
    const int components = TypeComponentCount(type);
    if (componentSize <= 0 || components <= 0) {
        if (outError) {
            *outError = "Unsupported accessor layout";
        }
        return false;
    }

    const size_t stride = byteStride != 0 ? byteStride : static_cast<size_t>(componentSize * components);
    const size_t required = accessorOffset + stride * (count - 1) + componentSize * components;
    if (byteLength != 0 && required > byteLength) {
        if (outError) {
            *outError = "Accessor extends beyond bufferView";
        }
        return false;
    }

    if (byteOffsetView + accessorOffset + componentSize * components > binData.size()) {
        if (outError) {
            *outError = "Accessor extends beyond BIN chunk";
        }
        return false;
    }

    outView->data = binData.data() + byteOffsetView + accessorOffset;
    outView->count = count;
    outView->stride = stride;
    outView->componentType = componentType;
    outView->type = type;
    return true;
}

bool ExtractAttribute(const AccessorView& view,
                      std::vector<float>* outValues,
                      int expectedComponents,
                      std::string* outError) {
    if (!outValues) {
        return false;
    }
    if (TypeComponentCount(view.type) != expectedComponents || view.componentType != 5126) {
        if (outError) {
            *outError = "Attribute accessor has unexpected format";
        }
        return false;
    }

    outValues->resize(view.count * static_cast<size_t>(expectedComponents));
    for (size_t i = 0; i < view.count; ++i) {
        const uint8_t* src = view.data + view.stride * i;
        for (int c = 0; c < expectedComponents; ++c) {
            float value = 0.0f;
            std::memcpy(&value, src + sizeof(float) * c, sizeof(float));
            (*outValues)[i * expectedComponents + c] = value;
        }
    }
    return true;
}

bool ExtractIndices(const AccessorView& view,
                    std::vector<uint32_t>* outValues,
                    std::string* outError) {
    if (!outValues) {
        return false;
    }
    if (TypeComponentCount(view.type) != 1) {
        if (outError) {
            *outError = "Index accessor must be scalar";
        }
        return false;
    }

    outValues->resize(view.count);
    for (size_t i = 0; i < view.count; ++i) {
        const uint8_t* src = view.data + view.stride * i;
        uint32_t value = 0;
        switch (view.componentType) {
            case 5121: {  // UNSIGNED_BYTE
                uint8_t tmp = 0;
                std::memcpy(&tmp, src, sizeof(uint8_t));
                value = tmp;
                break;
            }
            case 5123: {  // UNSIGNED_SHORT
                uint16_t tmp = 0;
                std::memcpy(&tmp, src, sizeof(uint16_t));
                value = tmp;
                break;
            }
            case 5125: {  // UNSIGNED_INT
                uint32_t tmp = 0;
                std::memcpy(&tmp, src, sizeof(uint32_t));
                value = tmp;
                break;
            }
            default:
                if (outError) {
                    *outError = "Unsupported index component type";
                }
                return false;
        }
        (*outValues)[i] = value;
    }
    return true;
}

}  // namespace

bool LoadMeshFromGlb(AAssetManager* assetManager,
                     const std::string& assetPath,
                     MeshSourceData* outData,
                     std::string* outError) {
    if (!assetManager || !outData) {
        if (outError) {
            *outError = "Invalid parameters";
        }
        return false;
    }

    std::vector<uint8_t> fileData;
    if (!ReadAssetFile(assetManager, assetPath, &fileData)) {
        if (outError) {
            *outError = "Unable to open asset: " + assetPath;
        }
        return false;
    }

    GlbChunks chunks;
    if (!ExtractGlbChunks(fileData, &chunks, outError)) {
        __android_log_print(ANDROID_LOG_WARN, kTag, "Failed to parse GLB '%s': %s", assetPath.c_str(),
                            outError ? outError->c_str() : "");
        return false;
    }

    JsonValue document;
    if (!ParseJson(chunks.json, &document)) {
        if (outError) {
            *outError = "JSON parse failure";
        }
        return false;
    }

    const JsonValue& meshes = document["meshes"];
    if (!meshes.IsArray() || meshes.Size() == 0) {
        if (outError) {
            *outError = "GLB contains no meshes";
        }
        return false;
    }

    const JsonValue& mesh = meshes[0];
    const JsonValue& primitives = mesh["primitives"];
    if (!primitives.IsArray() || primitives.Size() == 0) {
        if (outError) {
            *outError = "Mesh contains no primitives";
        }
        return false;
    }

    const JsonValue& primitive = primitives[0];
    const JsonValue& attributes = primitive["attributes"];
    if (!attributes.IsObject()) {
        if (outError) {
            *outError = "Primitive missing attributes";
        }
        return false;
    }

    const int positionAccessor = static_cast<int>(attributes["POSITION"].AsNumber(-1));
    if (positionAccessor < 0) {
        if (outError) {
            *outError = "Primitive missing POSITION attribute";
        }
        return false;
    }

    AccessorView positionView;
    if (!ResolveAccessor(document, positionAccessor, chunks.bin, &positionView, outError)) {
        return false;
    }

    std::vector<float> positions;
    if (!ExtractAttribute(positionView, &positions, 3, outError)) {
        return false;
    }

    std::vector<float> normals;
    const int normalAccessor = static_cast<int>(attributes["NORMAL"].AsNumber(-1));
    if (normalAccessor >= 0) {
        AccessorView normalView;
        if (ResolveAccessor(document, normalAccessor, chunks.bin, &normalView, outError)) {
            ExtractAttribute(normalView, &normals, 3, outError);
        }
    }

    std::vector<uint32_t> indices;
    const int indexAccessor = static_cast<int>(primitive["indices"].AsNumber(-1));
    if (indexAccessor >= 0) {
        AccessorView indexView;
        if (ResolveAccessor(document, indexAccessor, chunks.bin, &indexView, outError)) {
            ExtractIndices(indexView, &indices, outError);
        }
    }

    outData->positions = std::move(positions);
    outData->normals = std::move(normals);
    outData->indices = std::move(indices);
    return true;
}

}  // namespace engine
