#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/core/assembly_types.h"

namespace engine {

class KinematicsSystem {
public:
    KinematicsSystem() = default;

    bool Initialize(const std::vector<PartAnchor>& anchors,
                    const std::vector<AssemblyConstraint>& constraints);

    std::vector<PartTransform> BuildDefaultPose() const;

    std::vector<PartTransform> SolveForAngle(float crankRadians);

    const std::vector<PartAnchor>& Anchors() const { return anchors_; }
    const std::vector<AssemblyConstraint>& Constraints() const { return constraints_; }
    size_t ConstraintCount() const { return constraints_.size(); }

private:
    struct RotatingPart {
        size_t anchorIndex{static_cast<size_t>(-1)};
        Vec3 axisOrigin{0.0f, 0.0f, 0.0f};
        Vec3 axisDirection{0.0f, 0.0f, 1.0f};
        float angleScale{1.0f};
        float angleOffset{0.0f};
        Mat4 defaultTransform{Mat4::Identity()};
    };

    struct SliderCrankData {
        bool valid{false};
        size_t crankIndex{static_cast<size_t>(-1)};
        size_t rodIndex{static_cast<size_t>(-1)};
        size_t pistonIndex{static_cast<size_t>(-1)};

        Vec3 crankOrigin{0.0f, 0.0f, 0.0f};
        Vec3 crankAxis{0.0f, 0.0f, 1.0f};
        Vec3 crankPerpX{1.0f, 0.0f, 0.0f};
        Vec3 crankPerpY{0.0f, 1.0f, 0.0f};
        float crankRadius{0.0f};
        float crankAxisOffset{0.0f};

        Mat4 crankDefault{Mat4::Identity()};
        Mat4 rodDefault{Mat4::Identity()};
        Mat4 rodDefaultNoTranslation{Mat4::Identity()};
        Mat4 pistonDefault{Mat4::Identity()};

        Vec3 rodSmallLocal{0.0f, 0.0f, 0.0f};
        Vec3 rodBigLocal{0.0f, -1.0f, 0.0f};
        Vec3 pistonLocal{0.0f, 0.0f, 0.0f};
        Vec3 pistonAxis{0.0f, 1.0f, 0.0f};

        Vec3 pistonBaseWorld{0.0f, 0.0f, 0.0f};
        float rodLength{0.0f};
        Vec3 rodAxisDefault{0.0f, 1.0f, 0.0f};
    };

    struct LinearSlider {
        bool valid{false};
        size_t anchorIndex{static_cast<size_t>(-1)};
        Mat4 defaultTransform{Mat4::Identity()};
        Vec3 axis{0.0f, 1.0f, 0.0f};
        Vec3 localPoint{0.0f, 0.0f, 0.0f};
        Vec3 basePoint{0.0f, 0.0f, 0.0f};
    };

    struct ValvetrainSet {
        bool valid{false};
        size_t pushrodIndex{static_cast<size_t>(-1)};
        size_t rockerIndex{static_cast<size_t>(-1)};
        size_t valveIndex{static_cast<size_t>(-1)};

        LinearSlider pushrodSlider;
        LinearSlider valveSlider;

        Mat4 rockerDefault{Mat4::Identity()};
        Vec3 pivotPoint{0.0f, 0.0f, 0.0f};
        Vec3 pivotAxis{1.0f, 0.0f, 0.0f};
        Vec3 rockerPushrodLocal{0.0f, 0.0f, 0.0f};
        Vec3 rockerValveLocal{0.0f, 0.0f, 0.0f};
        Vec3 pushrodContactLocal{0.0f, 0.0f, 0.0f};
        Vec3 valveContactLocal{0.0f, 0.0f, 0.0f};
        float camPhase{0.0f};
        float pushrodAmplitude{0.0f};
        float pushrodDirection{1.0f};
    };

    struct RelativeFollower {
        size_t sourceIndex{static_cast<size_t>(-1)};
        size_t followerIndex{static_cast<size_t>(-1)};
        Mat4 relative{Mat4::Identity()};
    };

    struct ValidationPair {
        size_t partA{static_cast<size_t>(-1)};
        size_t partB{static_cast<size_t>(-1)};
        Vec3 localPointA{0.0f, 0.0f, 0.0f};
        Vec3 localPointB{0.0f, 0.0f, 0.0f};
        Vec3 localAxisA{0.0f, 0.0f, 0.0f};
        Vec3 localAxisB{0.0f, 0.0f, 0.0f};
    };

    void BuildDefaultPoseCache();
    void BuildSliderCrankData();
    void BuildRotatingParts();
    void BuildValvetrainData();
    void BuildFollowers();
    void BuildValidationPairs();
    void ApplySliderCrank(float crankRadians, std::vector<PartTransform>& transforms);
    void ApplyRotatingParts(float crankRadians, std::vector<PartTransform>& transforms) const;
    void ApplyValvetrain(float crankRadians, std::vector<PartTransform>& transforms);
    void ApplyFollowers(std::vector<PartTransform>& transforms) const;
    void ApplySliderDisplacement(const LinearSlider& slider, float displacement,
                                 std::vector<PartTransform>& transforms) const;
    void ValidateKeyPairs(const std::vector<PartTransform>& transforms) const;

    const PartTransform* FindPartTransform(const std::vector<PartTransform>& transforms,
                                           size_t index) const;

    std::vector<PartAnchor> anchors_;
    std::vector<AssemblyConstraint> constraints_;
    std::unordered_map<std::string, size_t> anchorLookup_;
    std::unordered_map<std::string, size_t> transformLookup_;
    std::vector<PartTransform> defaultPose_;
    std::vector<RotatingPart> rotatingParts_;
    SliderCrankData sliderCrank_;
    ValvetrainSet intakeTrain_;
    ValvetrainSet exhaustTrain_;
    std::vector<RelativeFollower> followers_;
    std::vector<ValidationPair> validationPairs_;
    float lastSliderDisplacement_{0.0f};
};

}  // namespace engine

