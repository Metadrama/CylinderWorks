#include "engine/core/kinematics_system.h"

#include <android/log.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace engine {
namespace {

constexpr const char* kTag = "EngineRenderer";
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647693f;

inline Vec3 TransformPoint(const Mat4& transform, const Vec3& point) {
    return Vec3{
        transform.data[0] * point.x + transform.data[4] * point.y + transform.data[8] * point.z + transform.data[12],
        transform.data[1] * point.x + transform.data[5] * point.y + transform.data[9] * point.z + transform.data[13],
        transform.data[2] * point.x + transform.data[6] * point.y + transform.data[10] * point.z + transform.data[14]
    };
}

inline Vec3 TransformDirection(const Mat4& transform, const Vec3& direction) {
    return Vec3{
        transform.data[0] * direction.x + transform.data[4] * direction.y + transform.data[8] * direction.z,
        transform.data[1] * direction.x + transform.data[5] * direction.y + transform.data[9] * direction.z,
        transform.data[2] * direction.x + transform.data[6] * direction.y + transform.data[10] * direction.z
    };
}

inline Mat4 RemoveTranslation(const Mat4& transform) {
    Mat4 result = transform;
    result.data[12] = 0.0f;
    result.data[13] = 0.0f;
    result.data[14] = 0.0f;
    return result;
}

inline Mat4 AxisAngleMatrix(const Vec3& axis, float angle) {
    const Vec3 normalized = Normalize(axis);
    const float len = Length(normalized);
    if (len <= std::numeric_limits<float>::epsilon()) {
        return Mat4::Identity();
    }

    const float x = normalized.x;
    const float y = normalized.y;
    const float z = normalized.z;
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const float t = 1.0f - c;

    Mat4 result = Mat4::Identity();
    result.data[0] = t * x * x + c;
    result.data[4] = t * x * y - s * z;
    result.data[8] = t * x * z + s * y;

    result.data[1] = t * x * y + s * z;
    result.data[5] = t * y * y + c;
    result.data[9] = t * y * z - s * x;

    result.data[2] = t * x * z - s * y;
    result.data[6] = t * y * z + s * x;
    result.data[10] = t * z * z + c;
    return result;
}

template <typename T>
bool HasValue(const T& value) {
    return value != static_cast<T>(-1);
}

}  // namespace

bool KinematicsSystem::Initialize(const std::vector<PartAnchor>& anchors,
                                  const std::vector<AssemblyConstraint>& constraints) {
    anchors_ = anchors;
    constraints_ = constraints;
    anchorLookup_.clear();
    transformLookup_.clear();
    rotatingParts_.clear();
    validationPairs_.clear();
    sliderCrank_ = SliderCrankData{};
    lastSliderDisplacement_ = 0.0f;

    for (size_t i = 0; i < anchors_.size(); ++i) {
        const std::string& name = anchors_[i].name;
        if (name.empty()) {
            __android_log_print(ANDROID_LOG_WARN, kTag,
                                "Kinematics anchor at index %zu is missing a name", i);
            continue;
        }
        anchorLookup_[name] = i;
        transformLookup_[name] = i;
    }

    BuildDefaultPoseCache();
    BuildSliderCrankData();
    BuildRotatingParts();
    BuildValvetrainData();
    BuildFollowers();
    BuildValidationPairs();

    return !anchors_.empty() || !constraints_.empty();
}

std::vector<PartTransform> KinematicsSystem::BuildDefaultPose() const {
    return defaultPose_;
}

std::vector<PartTransform> KinematicsSystem::SolveForAngle(float crankRadians) {
    std::vector<PartTransform> transforms = defaultPose_;
    if (transforms.empty()) {
        return transforms;
    }

    ApplySliderCrank(crankRadians, transforms);
    ApplyRotatingParts(crankRadians, transforms);
    ApplyValvetrain(crankRadians, transforms);
    ApplyFollowers(transforms);
    ValidateKeyPairs(transforms);
    return transforms;
}

void KinematicsSystem::BuildDefaultPoseCache() {
    defaultPose_.clear();
    defaultPose_.reserve(anchors_.size());
    for (const auto& anchor : anchors_) {
        PartTransform transform;
        transform.name = anchor.name;
        transform.transform = anchor.defaultTransform;
        defaultPose_.push_back(transform);
    }
}

void KinematicsSystem::BuildSliderCrankData() {
    sliderCrank_ = SliderCrankData{};
    lastSliderDisplacement_ = 0.0f;

    auto findIndex = [&](std::string_view name) -> size_t {
        auto it = anchorLookup_.find(std::string{name});
        if (it == anchorLookup_.end()) {
            return static_cast<size_t>(-1);
        }
        return it->second;
    };

    sliderCrank_.crankIndex = findIndex("crankshaft");
    sliderCrank_.rodIndex = findIndex("connecting_rod");
    sliderCrank_.pistonIndex = findIndex("piston");

    if (!HasValue(sliderCrank_.crankIndex) || !HasValue(sliderCrank_.rodIndex) ||
        !HasValue(sliderCrank_.pistonIndex)) {
        return;
    }

    auto findConcentric = [&](const std::string& partA, const std::string& partB,
                              ConstraintGeometry* outA, ConstraintGeometry* outB) -> bool {
        for (const auto& constraint : constraints_) {
            if (constraint.type != "Concentric") {
                continue;
            }
            const ConstraintGeometry* a = nullptr;
            const ConstraintGeometry* b = nullptr;
            for (const auto& geometry : constraint.geometries) {
                if (!partA.empty() && geometry.partName == partA) {
                    a = &geometry;
                } else if (!partB.empty() && geometry.partName == partB) {
                    b = &geometry;
                } else if (partB.empty() && geometry.ground) {
                    b = &geometry;
                }
            }
            if (a && b) {
                if (outA) {
                    *outA = *a;
                }
                if (outB) {
                    *outB = *b;
                }
                return true;
            }
        }
        return false;
    };

    ConstraintGeometry crankGeom;
    ConstraintGeometry crankGroundGeom;
    if (!findConcentric("crankshaft", std::string{}, &crankGeom, &crankGroundGeom)) {
        __android_log_print(ANDROID_LOG_WARN, kTag,
                            "Failed to locate crankshaft concentric constraint with ground");
        return;
    }

    ConstraintGeometry rodToPistonRodGeom;
    ConstraintGeometry rodToPistonPistonGeom;
    if (!findConcentric("connecting_rod", "piston", &rodToPistonRodGeom, &rodToPistonPistonGeom)) {
        __android_log_print(ANDROID_LOG_WARN, kTag,
                            "Failed to locate connecting rod to piston constraint");
        return;
    }

    ConstraintGeometry rodBigGeom;
    bool foundRodBig = false;
    for (const auto& constraint : constraints_) {
        if (constraint.type != "Concentric") {
            continue;
        }
        const ConstraintGeometry* rodGeom = nullptr;
        const ConstraintGeometry* other = nullptr;
        for (const auto& geometry : constraint.geometries) {
            if (geometry.partName == "connecting_rod") {
                rodGeom = &geometry;
            } else if (!geometry.partName.empty()) {
                other = &geometry;
            }
        }
        if (rodGeom && other && other->partName != "piston") {
            rodBigGeom = *rodGeom;
            foundRodBig = true;
            break;
        }
    }

    if (!foundRodBig) {
        rodBigGeom = rodToPistonRodGeom;
    }

    sliderCrank_.crankDefault = anchors_[sliderCrank_.crankIndex].defaultTransform;
    sliderCrank_.rodDefault = anchors_[sliderCrank_.rodIndex].defaultTransform;
    sliderCrank_.rodDefaultNoTranslation = RemoveTranslation(sliderCrank_.rodDefault);
    sliderCrank_.pistonDefault = anchors_[sliderCrank_.pistonIndex].defaultTransform;

    sliderCrank_.rodSmallLocal = rodToPistonRodGeom.position;
    sliderCrank_.rodBigLocal = rodBigGeom.position;
    sliderCrank_.pistonLocal = rodToPistonPistonGeom.position;

    sliderCrank_.pistonAxis = Vec3{0.0f, 1.0f, 0.0f};
    ConstraintGeometry pistonGroundGeom;
    if (findConcentric("piston", std::string{}, &pistonGroundGeom, nullptr)) {
        sliderCrank_.pistonAxis = Normalize(
            TransformDirection(sliderCrank_.pistonDefault, pistonGroundGeom.axis));
    } else {
        sliderCrank_.pistonAxis = Normalize(
            TransformDirection(sliderCrank_.pistonDefault, rodToPistonPistonGeom.axis));
    }
    if (Length(sliderCrank_.pistonAxis) <= std::numeric_limits<float>::epsilon()) {
        sliderCrank_.pistonAxis = Vec3{0.0f, 1.0f, 0.0f};
    }

    const Vec3 rodSmallWorld = TransformPoint(sliderCrank_.rodDefault, sliderCrank_.rodSmallLocal);
    const Vec3 rodBigWorld = TransformPoint(sliderCrank_.rodDefault, sliderCrank_.rodBigLocal);
    sliderCrank_.pistonBaseWorld = TransformPoint(sliderCrank_.pistonDefault, sliderCrank_.pistonLocal);

    sliderCrank_.rodLength = Length(rodBigWorld - rodSmallWorld);
    if (sliderCrank_.rodLength <= std::numeric_limits<float>::epsilon()) {
        __android_log_print(ANDROID_LOG_WARN, kTag,
                            "Connecting rod length is zero; cannot solve slider-crank");
        return;
    }

    sliderCrank_.rodAxisDefault = Normalize(rodBigWorld - rodSmallWorld);

    sliderCrank_.crankOrigin = TransformPoint(sliderCrank_.crankDefault, crankGeom.position);
    sliderCrank_.crankAxis = Normalize(TransformDirection(sliderCrank_.crankDefault, crankGeom.axis));
    if (Length(sliderCrank_.crankAxis) <= std::numeric_limits<float>::epsilon()) {
        sliderCrank_.crankAxis = Vec3{1.0f, 0.0f, 0.0f};
    }

    const Vec3 toBig = rodBigWorld - sliderCrank_.crankOrigin;
    sliderCrank_.crankAxisOffset = Dot(toBig, sliderCrank_.crankAxis);
    Vec3 radial = toBig - sliderCrank_.crankAxis * sliderCrank_.crankAxisOffset;
    sliderCrank_.crankRadius = Length(radial);
    if (sliderCrank_.crankRadius <= std::numeric_limits<float>::epsilon()) {
        __android_log_print(ANDROID_LOG_WARN, kTag,
                            "Crank radius is zero; cannot animate crankshaft");
        return;
    }

    sliderCrank_.crankPerpX = radial / sliderCrank_.crankRadius;
    sliderCrank_.crankPerpY = Normalize(Cross(sliderCrank_.crankAxis, sliderCrank_.crankPerpX));
    if (Length(sliderCrank_.crankPerpY) <= std::numeric_limits<float>::epsilon()) {
        sliderCrank_.crankPerpY = Normalize(Cross(sliderCrank_.crankAxis, Vec3{0.0f, 1.0f, 0.0f}));
    }

    sliderCrank_.valid = true;
}

void KinematicsSystem::BuildRotatingParts() {
    rotatingParts_.clear();

    const std::unordered_map<std::string, float> kRatios{
        {"propeller", 0.5f},
        {"driving_gear", 1.0f},
        {"gear", -1.0f},
        {"camshaft", 0.5f},
    };

    auto findAxisForPart = [&](const std::string& name, Vec3* origin, Vec3* axis) -> bool {
        ConstraintGeometry partGeom;
        bool found = false;
        for (const auto& constraint : constraints_) {
            if (constraint.type != "Concentric") {
                continue;
            }
            for (const auto& geometry : constraint.geometries) {
                if (geometry.partName == name) {
                    partGeom = geometry;
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }

        if (!found) {
            return false;
        }

        auto it = anchorLookup_.find(name);
        if (it == anchorLookup_.end()) {
            return false;
        }

        const Mat4& defaultTransform = anchors_[it->second].defaultTransform;
        *origin = TransformPoint(defaultTransform, partGeom.position);
        *axis = Normalize(TransformDirection(defaultTransform, partGeom.axis));
        if (Length(*axis) <= std::numeric_limits<float>::epsilon()) {
            *axis = Vec3{1.0f, 0.0f, 0.0f};
        }
        return true;
    };

    for (const auto& entry : kRatios) {
        auto it = anchorLookup_.find(entry.first);
        if (it == anchorLookup_.end()) {
            continue;
        }

        RotatingPart part;
        part.anchorIndex = it->second;
        part.defaultTransform = anchors_[part.anchorIndex].defaultTransform;
        part.angleScale = entry.second;

        Vec3 origin{0.0f, 0.0f, 0.0f};
        Vec3 axis{1.0f, 0.0f, 0.0f};
        if (!findAxisForPart(entry.first, &origin, &axis)) {
            origin = Vec3{part.defaultTransform.data[12], part.defaultTransform.data[13],
                          part.defaultTransform.data[14]};
            axis = Vec3{1.0f, 0.0f, 0.0f};
        }
        part.axisOrigin = origin;
        part.axisDirection = axis;
        rotatingParts_.push_back(part);
    }
}

void KinematicsSystem::BuildValvetrainData() {
    intakeTrain_ = ValvetrainSet{};
    exhaustTrain_ = ValvetrainSet{};

    auto findIndex = [&](const std::string& name) -> size_t {
        auto it = anchorLookup_.find(name);
        if (it == anchorLookup_.end()) {
            return static_cast<size_t>(-1);
        }
        return it->second;
    };

    auto findConcentricWithGround = [&](const std::string& part, ConstraintGeometry* partGeom,
                                        ConstraintGeometry* groundGeom) -> bool {
        for (const auto& constraint : constraints_) {
            if (constraint.type != "Concentric") {
                continue;
            }
            const ConstraintGeometry* localPart = nullptr;
            const ConstraintGeometry* localGround = nullptr;
            for (const auto& geometry : constraint.geometries) {
                if (geometry.partName == part) {
                    localPart = &geometry;
                } else if (geometry.ground) {
                    localGround = &geometry;
                }
            }
            if (localPart && localGround) {
                if (partGeom) {
                    *partGeom = *localPart;
                }
                if (groundGeom) {
                    *groundGeom = *localGround;
                }
                return true;
            }
        }
        return false;
    };

    auto findConstraintBetween = [&](const std::string& type, const std::string& partA,
                                     const std::string& partB, ConstraintGeometry* geomA,
                                     ConstraintGeometry* geomB) -> bool {
        for (const auto& constraint : constraints_) {
            if (constraint.type != type) {
                continue;
            }
            const ConstraintGeometry* localA = nullptr;
            const ConstraintGeometry* localB = nullptr;
            for (const auto& geometry : constraint.geometries) {
                if (geometry.partName == partA) {
                    localA = &geometry;
                } else if (geometry.partName == partB) {
                    localB = &geometry;
                }
            }
            if (localA && localB) {
                if (geomA) {
                    *geomA = *localA;
                }
                if (geomB) {
                    *geomB = *localB;
                }
                return true;
            }
        }
        return false;
    };

    auto buildTrain = [&](ValvetrainSet* train, const std::string& pushrodName,
                          const std::string& rockerName, const std::string& valveName,
                          float phase) {
        if (!train) {
            return;
        }

        const size_t pushrodIndex = findIndex(pushrodName);
        const size_t rockerIndex = findIndex(rockerName);
        const size_t valveIndex = findIndex(valveName);
        if (!HasValue(pushrodIndex) || !HasValue(rockerIndex) || !HasValue(valveIndex)) {
            return;
        }

        train->pushrodIndex = pushrodIndex;
        train->rockerIndex = rockerIndex;
        train->valveIndex = valveIndex;
        train->camPhase = phase;

        train->pushrodSlider.anchorIndex = pushrodIndex;
        train->pushrodSlider.defaultTransform = anchors_[pushrodIndex].defaultTransform;
        train->valveSlider.anchorIndex = valveIndex;
        train->valveSlider.defaultTransform = anchors_[valveIndex].defaultTransform;
        train->rockerDefault = anchors_[rockerIndex].defaultTransform;

        ConstraintGeometry pushrodGeom;
        if (!findConcentricWithGround(pushrodName, &pushrodGeom, nullptr)) {
            return;
        }

        ConstraintGeometry valveGeom;
        if (!findConcentricWithGround(valveName, &valveGeom, nullptr)) {
            return;
        }

        ConstraintGeometry rockerPivot;
        if (!findConstraintBetween("Concentric", rockerName, "shaft", &rockerPivot, nullptr)) {
            return;
        }

        ConstraintGeometry pushrodContact;
        ConstraintGeometry rockerPushrodContact;
        if (!findConstraintBetween("Tangent", pushrodName, rockerName,
                                   &pushrodContact, &rockerPushrodContact)) {
            return;
        }

        ConstraintGeometry rockerValveContact;
        ConstraintGeometry valveContact;
        if (!findConstraintBetween("Tangent", rockerName, valveName,
                                   &rockerValveContact, &valveContact)) {
            return;
        }

        train->pushrodSlider.localPoint = pushrodGeom.position;
        train->pushrodSlider.basePoint =
            TransformPoint(train->pushrodSlider.defaultTransform, pushrodGeom.position);
        train->pushrodSlider.axis = Normalize(
            TransformDirection(train->pushrodSlider.defaultTransform, pushrodGeom.axis));
        if (Length(train->pushrodSlider.axis) <= std::numeric_limits<float>::epsilon()) {
            train->pushrodSlider.axis = Vec3{0.0f, 1.0f, 0.0f};
        }
        train->pushrodSlider.valid = true;

        train->valveSlider.localPoint = valveGeom.position;
        train->valveSlider.basePoint =
            TransformPoint(train->valveSlider.defaultTransform, valveGeom.position);
        train->valveSlider.axis = Normalize(
            TransformDirection(train->valveSlider.defaultTransform, valveGeom.axis));
        if (Length(train->valveSlider.axis) <= std::numeric_limits<float>::epsilon()) {
            train->valveSlider.axis = Vec3{0.0f, 1.0f, 0.0f};
        }
        train->valveSlider.valid = true;

        train->pivotPoint = TransformPoint(train->rockerDefault, rockerPivot.position);
        train->pivotAxis = Normalize(
            TransformDirection(train->rockerDefault, rockerPivot.axis));
        if (Length(train->pivotAxis) <= std::numeric_limits<float>::epsilon()) {
            train->pivotAxis = Vec3{1.0f, 0.0f, 0.0f};
        }

        train->rockerPushrodLocal = rockerPushrodContact.position;
        train->rockerValveLocal = rockerValveContact.position;
        train->pushrodContactLocal = pushrodContact.position;
        train->valveContactLocal = valveContact.position;

        const Vec3 pivotAxis = train->pivotAxis;
        const Vec3 pivotToPushrod =
            TransformPoint(train->rockerDefault, train->rockerPushrodLocal) - train->pivotPoint;
        const Vec3 pivotToValve =
            TransformPoint(train->rockerDefault, train->rockerValveLocal) - train->pivotPoint;

        const Vec3 pushrodAxis = train->pushrodSlider.axis;
        const Vec3 valveAxis = train->valveSlider.axis;

        const Vec3 pushrodMomentArm = Cross(pivotAxis, pivotToPushrod);
        const Vec3 valveMomentArm = Cross(pivotAxis, pivotToValve);

        const float pushrodEff = Dot(pushrodMomentArm, pushrodAxis);
        const float valveEff = Dot(valveMomentArm, valveAxis);

        constexpr float kDesiredValveLift = 0.012f;
        train->pushrodAmplitude = 0.0f;
        train->pushrodDirection = 1.0f;
        if (std::fabs(valveEff) > 1e-5f && std::fabs(pushrodEff) > 1e-5f) {
            const float ratio = pushrodEff / valveEff;
            float amplitude = kDesiredValveLift * ratio;
            if (amplitude < 0.0f) {
                train->pushrodDirection = -1.0f;
                amplitude = -amplitude;
            }
            train->pushrodAmplitude = amplitude;
        }

        if (train->pushrodAmplitude <= 1e-5f) {
            train->pushrodAmplitude = 0.008f;
            train->pushrodDirection = 1.0f;
        }

        train->valid = true;
    };

    buildTrain(&intakeTrain_, "rod", "rocker_arm", "valve", 0.0f);
    buildTrain(&exhaustTrain_, "rod_2", "rocker_arm_2", "valve_2", kPi);
}

void KinematicsSystem::BuildFollowers() {
    followers_.clear();

    struct Pair {
        const char* source;
        const char* follower;
    };

    const Pair pairs[] = {
        {"connecting_rod", "cover"},
        {"connecting_rod", "pin"},
        {"valve", "retainer"},
        {"valve", "valve_spring_retainer"},
        {"valve_2", "retainer_2"},
        {"valve_2", "valve_spring_retainer_2"},
    };

    for (const auto& pair : pairs) {
        auto sourceIt = anchorLookup_.find(pair.source);
        auto followerIt = anchorLookup_.find(pair.follower);
        if (sourceIt == anchorLookup_.end() || followerIt == anchorLookup_.end()) {
            continue;
        }

        RelativeFollower follower;
        follower.sourceIndex = sourceIt->second;
        follower.followerIndex = followerIt->second;

        const Mat4 sourceInv = InvertRigidTransform(anchors_[follower.sourceIndex].defaultTransform);
        follower.relative = Multiply(sourceInv, anchors_[follower.followerIndex].defaultTransform);
        followers_.push_back(follower);
    }
}

void KinematicsSystem::BuildValidationPairs() {
    validationPairs_.clear();
    const std::unordered_set<std::string> kWatched{
        "connecting_rod", "piston", "valve", "valve_2"
    };

    for (const auto& constraint : constraints_) {
        if (constraint.type != "Concentric" && constraint.type != "Coincident") {
            continue;
        }

        std::vector<const ConstraintGeometry*> entries;
        for (const auto& geometry : constraint.geometries) {
            if (geometry.ground || geometry.partName.empty()) {
                continue;
            }
            entries.push_back(&geometry);
        }

        for (size_t i = 0; i < entries.size(); ++i) {
            for (size_t j = i + 1; j < entries.size(); ++j) {
                const auto* first = entries[i];
                const auto* second = entries[j];
                if (!kWatched.count(first->partName) && !kWatched.count(second->partName)) {
                    continue;
                }

                auto firstIndex = transformLookup_.find(first->partName);
                auto secondIndex = transformLookup_.find(second->partName);
                if (firstIndex == transformLookup_.end() || secondIndex == transformLookup_.end()) {
                    continue;
                }

                ValidationPair pair;
                pair.partA = firstIndex->second;
                pair.partB = secondIndex->second;
                pair.localPointA = first->position;
                pair.localPointB = second->position;
                pair.localAxisA = first->axis;
                pair.localAxisB = second->axis;
                validationPairs_.push_back(pair);
            }
        }
    }
}

void KinematicsSystem::ApplySliderCrank(float crankRadians,
                                        std::vector<PartTransform>& transforms) {
    if (!sliderCrank_.valid) {
        return;
    }

    if (sliderCrank_.crankIndex >= transforms.size() ||
        sliderCrank_.rodIndex >= transforms.size() ||
        sliderCrank_.pistonIndex >= transforms.size()) {
        return;
    }

    const Vec3 bigEnd = sliderCrank_.crankOrigin +
                        sliderCrank_.crankAxis * sliderCrank_.crankAxisOffset +
                        sliderCrank_.crankPerpX * (sliderCrank_.crankRadius * std::cos(crankRadians)) +
                        sliderCrank_.crankPerpY * (sliderCrank_.crankRadius * std::sin(crankRadians));

    const Vec3 relative = bigEnd - sliderCrank_.pistonBaseWorld;
    const float dotAxis = Dot(sliderCrank_.pistonAxis, relative);
    const float relSq = Dot(relative, relative);
    const float c = relSq - sliderCrank_.rodLength * sliderCrank_.rodLength;
    float discriminant = dotAxis * dotAxis - c;
    if (discriminant < 0.0f) {
        discriminant = 0.0f;
    }
    const float root = std::sqrt(discriminant);
    float candidate1 = dotAxis + root;
    float candidate2 = dotAxis - root;

    if (!std::isfinite(candidate1)) {
        candidate1 = candidate2;
    }
    if (!std::isfinite(candidate2)) {
        candidate2 = candidate1;
    }

    float displacement = candidate1;
    if (std::fabs(candidate2 - lastSliderDisplacement_) < std::fabs(candidate1 - lastSliderDisplacement_)) {
        displacement = candidate2;
    }
    lastSliderDisplacement_ = displacement;

    const Vec3 smallEnd = sliderCrank_.pistonBaseWorld + sliderCrank_.pistonAxis * displacement;

    // Update piston transform.
    const Vec3 pistonDelta = smallEnd - sliderCrank_.pistonBaseWorld;
    Mat4 pistonTranslation = Translation(pistonDelta);
    transforms[sliderCrank_.pistonIndex].transform = Multiply(pistonTranslation, sliderCrank_.pistonDefault);

    // Update connecting rod transform.
    const Vec3 targetAxis = Normalize(bigEnd - smallEnd);
    Vec3 rotationAxis = Cross(sliderCrank_.rodAxisDefault, targetAxis);
    const float axisLength = Length(rotationAxis);
    const float dotAxisRod = Clamp(Dot(sliderCrank_.rodAxisDefault, targetAxis), -1.0f, 1.0f);

    Mat4 rotationMatrix = Mat4::Identity();
    if (axisLength > 1e-6f) {
        rotationAxis = rotationAxis / axisLength;
        const float angle = std::atan2(axisLength, dotAxisRod);
        rotationMatrix = AxisAngleMatrix(rotationAxis, angle);
    } else if (dotAxisRod < 0.0f) {
        rotationMatrix = AxisAngleMatrix(sliderCrank_.crankAxis, 3.14159265358979323846f);
    }

    Mat4 rotatedRod = Multiply(rotationMatrix, sliderCrank_.rodDefaultNoTranslation);
    const Vec3 rotatedSmall = TransformPoint(rotatedRod, sliderCrank_.rodSmallLocal);
    const Vec3 translation = smallEnd - rotatedSmall;
    Mat4 rodTranslation = Translation(translation);
    transforms[sliderCrank_.rodIndex].transform = Multiply(rodTranslation, rotatedRod);

    // Update crankshaft transform.
    Mat4 translateToOrigin = Translation(sliderCrank_.crankOrigin * -1.0f);
    Mat4 translateBack = Translation(sliderCrank_.crankOrigin);
    Mat4 rotation = AxisAngleMatrix(sliderCrank_.crankAxis, crankRadians);
    Mat4 combined = Multiply(translateBack, Multiply(rotation, Multiply(translateToOrigin, sliderCrank_.crankDefault)));
    transforms[sliderCrank_.crankIndex].transform = combined;
}

void KinematicsSystem::ApplyRotatingParts(float crankRadians,
                                          std::vector<PartTransform>& transforms) const {
    for (const auto& part : rotatingParts_) {
        if (part.anchorIndex >= transforms.size()) {
            continue;
        }

        const float angle = crankRadians * part.angleScale + part.angleOffset;
        Mat4 translateToOrigin = Translation(part.axisOrigin * -1.0f);
        Mat4 translateBack = Translation(part.axisOrigin);
        Mat4 rotation = AxisAngleMatrix(part.axisDirection, angle);
        Mat4 combined = Multiply(translateBack, Multiply(rotation, Multiply(translateToOrigin, part.defaultTransform)));
        transforms[part.anchorIndex].transform = combined;
    }
}

void KinematicsSystem::ApplySliderDisplacement(const LinearSlider& slider, float displacement,
                                               std::vector<PartTransform>& transforms) const {
    if (!slider.valid || slider.anchorIndex >= transforms.size()) {
        return;
    }

    const Vec3 translation = slider.axis * displacement;
    const Mat4 translationMatrix = Translation(translation);
    transforms[slider.anchorIndex].transform =
        Multiply(translationMatrix, slider.defaultTransform);
}

void KinematicsSystem::ApplyValvetrain(float crankRadians, std::vector<PartTransform>& transforms) {
    auto solveTrain = [&](ValvetrainSet& train) {
        if (!train.valid) {
            return;
        }
        if (train.pushrodIndex >= transforms.size() || train.rockerIndex >= transforms.size() ||
            train.valveIndex >= transforms.size()) {
            return;
        }

        float camAngle = std::fmod(crankRadians * 0.5f + train.camPhase, kTwoPi);
        if (camAngle < 0.0f) {
            camAngle += kTwoPi;
        }

        constexpr float kOpenDuration = kPi;
        float normalized = 0.0f;
        if (camAngle < kOpenDuration) {
            const float progress = camAngle / kOpenDuration;
            normalized = 0.5f * (1.0f - std::cos(progress * kPi));
        }

        const float pushrodDisplacement = train.pushrodAmplitude * normalized * train.pushrodDirection;
        ApplySliderDisplacement(train.pushrodSlider, pushrodDisplacement, transforms);

        const Mat4& pushrodTransform = transforms[train.pushrodIndex].transform;
        const Vec3 pushrodContactWorld =
            TransformPoint(pushrodTransform, train.pushrodContactLocal);

        Vec3 pivotAxis = train.pivotAxis;
        if (Length(pivotAxis) <= std::numeric_limits<float>::epsilon()) {
            return;
        }
        pivotAxis = Normalize(pivotAxis);

        const Vec3 defaultPushrodWorld =
            TransformPoint(train.rockerDefault, train.rockerPushrodLocal);
        const Vec3 defaultVector = defaultPushrodWorld - train.pivotPoint;
        const Vec3 targetVector = pushrodContactWorld - train.pivotPoint;

        Vec3 projectedDefault = defaultVector - pivotAxis * Dot(pivotAxis, defaultVector);
        Vec3 projectedTarget = targetVector - pivotAxis * Dot(pivotAxis, targetVector);

        const float defaultLength = Length(projectedDefault);
        const float targetLength = Length(projectedTarget);
        if (defaultLength <= 1e-5f || targetLength <= 1e-5f) {
            return;
        }

        projectedDefault = projectedDefault / defaultLength;
        projectedTarget = projectedTarget / targetLength;

        const Vec3 crossVec = Cross(projectedDefault, projectedTarget);
        const float sinAngle = Dot(pivotAxis, crossVec);
        const float cosAngle = Clamp(Dot(projectedDefault, projectedTarget), -1.0f, 1.0f);
        const float angle = std::atan2(sinAngle, cosAngle);

        const Mat4 translateToPivot = Translation(train.pivotPoint * -1.0f);
        const Mat4 translateBack = Translation(train.pivotPoint);
        const Mat4 rotation = AxisAngleMatrix(pivotAxis, angle);
        const Mat4 updatedRocker =
            Multiply(translateBack, Multiply(rotation, Multiply(translateToPivot, train.rockerDefault)));
        transforms[train.rockerIndex].transform = updatedRocker;

        const Vec3 rockerPushrodContact =
            TransformPoint(updatedRocker, train.rockerPushrodLocal);
        const float correctedPushrodDisplacement =
            Dot(rockerPushrodContact - train.pushrodSlider.basePoint, train.pushrodSlider.axis);
        ApplySliderDisplacement(train.pushrodSlider, correctedPushrodDisplacement, transforms);

        const Vec3 rockerValveContact =
            TransformPoint(updatedRocker, train.rockerValveLocal);
        float valveDisplacement =
            Dot(rockerValveContact - train.valveSlider.basePoint, train.valveSlider.axis);
        if (valveDisplacement < 0.0f) {
            valveDisplacement = 0.0f;
        }
        ApplySliderDisplacement(train.valveSlider, valveDisplacement, transforms);
    };

    solveTrain(intakeTrain_);
    solveTrain(exhaustTrain_);
}

void KinematicsSystem::ApplyFollowers(std::vector<PartTransform>& transforms) const {
    for (const auto& follower : followers_) {
        if (follower.sourceIndex >= transforms.size() ||
            follower.followerIndex >= transforms.size()) {
            continue;
        }
        transforms[follower.followerIndex].transform =
            Multiply(transforms[follower.sourceIndex].transform, follower.relative);
    }
}

void KinematicsSystem::ValidateKeyPairs(const std::vector<PartTransform>& transforms) const {
    constexpr float kPositionTolerance = 1e-3f;
    constexpr float kAxisTolerance = 0.99f;

    for (const auto& pair : validationPairs_) {
        const PartTransform* transformA = FindPartTransform(transforms, pair.partA);
        const PartTransform* transformB = FindPartTransform(transforms, pair.partB);
        if (!transformA || !transformB) {
            continue;
        }

        const Vec3 worldA = TransformPoint(transformA->transform, pair.localPointA);
        const Vec3 worldB = TransformPoint(transformB->transform, pair.localPointB);
        const float distance = Length(worldA - worldB);

        float axisAlignment = 1.0f;
        if (Length(pair.localAxisA) > std::numeric_limits<float>::epsilon() &&
            Length(pair.localAxisB) > std::numeric_limits<float>::epsilon()) {
            const Vec3 axisA = Normalize(TransformDirection(transformA->transform, pair.localAxisA));
            const Vec3 axisB = Normalize(TransformDirection(transformB->transform, pair.localAxisB));
            axisAlignment = std::fabs(Dot(axisA, axisB));
        }

        if (distance > kPositionTolerance || axisAlignment < kAxisTolerance) {
            __android_log_print(ANDROID_LOG_ERROR, kTag,
                                "Constraint between '%s' and '%s' violated (distance %.6f, axis %.6f)",
                                anchors_[pair.partA].name.c_str(), anchors_[pair.partB].name.c_str(),
                                distance, axisAlignment);
        }
    }
}

const PartTransform* KinematicsSystem::FindPartTransform(
    const std::vector<PartTransform>& transforms, size_t index) const {
    if (index >= transforms.size()) {
        return nullptr;
    }
    return &transforms[index];
}

}  // namespace engine

