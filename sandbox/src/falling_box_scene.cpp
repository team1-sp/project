#include "falling_box_scene.hpp"

#include <cstdint>

#include <box2d/box2d.h>

namespace sandbox {

namespace {

void* toUserData(SpikeEntityId entity) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity));
}

SpikeEntityId entityOfShape(b2ShapeId shape) {
    const auto raw = reinterpret_cast<std::uintptr_t>(b2Body_GetUserData(b2Shape_GetBody(shape)));
    return static_cast<SpikeEntityId>(raw);
}

BodyPose poseOf(b2BodyId body) {
    const b2Vec2 position = b2Body_GetPosition(body);
    return BodyPose{position.x, position.y, b2Rot_GetAngle(b2Body_GetRotation(body))};
}

} // namespace

// Box2D v3 hands out ids (small value handles), not pointers. They stay in this file.
struct FallingBoxScene::Ids {
    b2WorldId world;
    b2BodyId ground;
    b2BodyId box;
};

FallingBoxScene::FallingBoxScene()
    : ids_(std::make_unique<Ids>()) {
    // Always start from the b2Default*Def() functions: a zero-initialised def fails Box2D's
    // validity check.
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{0.0f, -10.0f};
    ids_->world = b2CreateWorld(&worldDef);

    // Ground: static body, its top surface at y = 1 m.
    b2BodyDef groundDef = b2DefaultBodyDef(); // static is the default type
    groundDef.position = b2Vec2{groundSize.widthMeters / 2.0f, groundSize.heightMeters / 2.0f};
    groundDef.userData = toUserData(groundEntity);
    ids_->ground = b2CreateBody(ids_->world, &groundDef);
    const b2Polygon groundBox =
        b2MakeBox(groundSize.widthMeters / 2.0f, groundSize.heightMeters / 2.0f);
    const b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(ids_->ground, &groundShapeDef, &groundBox);

    // Box: dynamic body dropped from 12 m, slightly tilted so it lands on a corner and turns.
    b2BodyDef boxDef = b2DefaultBodyDef();
    boxDef.type = b2_dynamicBody;
    boxDef.position = b2Vec2{groundSize.widthMeters / 2.0f, 12.0f};
    boxDef.rotation = b2MakeRot(0.3f);
    boxDef.userData = toUserData(boxEntity);
    ids_->box = b2CreateBody(ids_->world, &boxDef);
    const b2Polygon box = b2MakeBox(boxSize.widthMeters / 2.0f, boxSize.heightMeters / 2.0f);
    b2ShapeDef boxShapeDef = b2DefaultShapeDef();
    boxShapeDef.density = 1.0f;
    boxShapeDef.material.friction = 0.6f;
    // Contact events are off by default in v3.1.1; one of the two shapes must turn them on.
    boxShapeDef.enableContactEvents = true;
    b2CreatePolygonShape(ids_->box, &boxShapeDef, &box);
}

FallingBoxScene::~FallingBoxScene() {
    // Destroying the world destroys every body and shape in it.
    b2DestroyWorld(ids_->world);
}

void FallingBoxScene::step() {
    b2World_Step(ids_->world, timeStepSeconds, subStepCount);

    // Events are only valid until the next step, so copy what we need now.
    const b2ContactEvents events = b2World_GetContactEvents(ids_->world);
    for (int i = 0; i < events.beginCount; ++i) {
        const b2ContactBeginTouchEvent& event = events.beginEvents[i];
        contactBegins_.push_back(
            ContactBegin{entityOfShape(event.shapeIdA), entityOfShape(event.shapeIdB)});
    }
}

BodyPose FallingBoxScene::groundPose() const {
    return poseOf(ids_->ground);
}

BodyPose FallingBoxScene::boxPose() const {
    return poseOf(ids_->box);
}

float FallingBoxScene::boxSpeedMetersPerSecond() const {
    return b2Length(b2Body_GetLinearVelocity(ids_->box));
}

bool FallingBoxScene::boxIsAwake() const {
    return b2Body_IsAwake(ids_->box);
}

const std::vector<ContactBegin>& FallingBoxScene::contactBegins() const {
    return contactBegins_;
}

} // namespace sandbox
