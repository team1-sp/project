#pragma once

#include <cstdint>
#include <memory>
#include <vector>

// PB-008 spike: a static ground and a dynamic box simulated by Box2D v3.1.1. Throwaway code to
// learn world, body, shape and step before the Physics Adapter is built. No Box2D type appears
// in this header, so the renderer and the tests only see our own types.
namespace sandbox {

// Stored in each body's userData, the way the adapter will map bodies back to entities.
using SpikeEntityId = std::uint32_t;
inline constexpr SpikeEntityId groundEntity = 1;
inline constexpr SpikeEntityId boxEntity = 2;

struct BodyPose {
    float xMeters;
    float yMeters;
    float angleRadians;
};

struct BoxSize {
    float widthMeters;
    float heightMeters;
};

struct ContactBegin {
    SpikeEntityId entityA;
    SpikeEntityId entityB;
};

class FallingBoxScene {
public:
    // Fixed step from R-5; 4 substeps is what the Box2D docs recommend.
    static constexpr float timeStepSeconds = 1.0f / 60.0f;
    static constexpr int subStepCount = 4;

    static constexpr BoxSize groundSize{25.0f, 1.0f};
    static constexpr BoxSize boxSize{1.0f, 1.0f};

    FallingBoxScene();
    ~FallingBoxScene();
    FallingBoxScene(const FallingBoxScene&) = delete;
    FallingBoxScene& operator=(const FallingBoxScene&) = delete;

    // Advances the world by one fixed step and collects the contact events of that step.
    void step();

    BodyPose groundPose() const;
    BodyPose boxPose() const;
    float boxSpeedMetersPerSecond() const;
    bool boxIsAwake() const;

    // Every begin-touch contact since the scene was created, in the order Box2D reported them.
    const std::vector<ContactBegin>& contactBegins() const;

private:
    struct Ids;
    std::unique_ptr<Ids> ids_;
    std::vector<ContactBegin> contactBegins_;
};

} // namespace sandbox
