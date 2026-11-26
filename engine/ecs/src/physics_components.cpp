#include "engine/ecs/physics_components.hpp"

#include "engine/ecs/type_registry.hpp"

namespace engine {

namespace {

// The fields every collider shares (srs.fr.3.7).
template <typename T>
void colliderFields(ComponentBuilder<T>& builder) {
    builder.field("offsetX", &T::offsetXMeters, Unit::Meters, "Shape centre, right of the body")
        .field("offsetY", &T::offsetYMeters, Unit::Meters, "Shape centre, above the body")
        .field("friction", &T::friction, Unit::None, "Friction, 0 for ice")
        .field("restitution", &T::restitution, Unit::None, "Bounciness: 0 none, 1 full bounce")
        .field("isSensor", &T::isSensor, Unit::None, "Detects overlaps but pushes nothing");
}

} // namespace

void registerPhysicsComponents(TypeRegistry& types) {
    types.enumeration<RigidBodyType>("RigidBodyType", "How a physics body moves")
        .value(RigidBodyType::Static, "Static", "Never moves")
        .value(RigidBodyType::Kinematic, "Kinematic", "Moves only at its Velocity")
        .value(RigidBodyType::Dynamic, "Dynamic", "Moved by gravity, forces and collisions");

    types.component<RigidBodyComponent>()
        .describe("How the entity's physics body moves; without it a collider's body is static.")
        .field("bodyType", &RigidBodyComponent::bodyType, Unit::None,
               "Static, Kinematic or Dynamic")
        .field("mass", &RigidBodyComponent::massKilograms, Unit::Kilograms,
               "Mass of a dynamic body")
        .field("gravityScale", &RigidBodyComponent::gravityScale, Unit::None,
               "Multiplies gravity; 0 floats")
        .field("linearDamping", &RigidBodyComponent::linearDamping, Unit::None,
               "Slows the body down over time")
        .field("fixedRotation", &RigidBodyComponent::fixedRotation, Unit::None,
               "The body never rotates");

    auto box = types.component<BoxColliderComponent>();
    box.describe("A box shape on the entity's physics body.")
        .field("width", &BoxColliderComponent::widthMeters, Unit::Meters, "Box width")
        .field("height", &BoxColliderComponent::heightMeters, Unit::Meters, "Box height");
    colliderFields(box);

    auto circle = types.component<CircleColliderComponent>();
    circle.describe("A circle shape on the entity's physics body.")
        .field("radius", &CircleColliderComponent::radiusMeters, Unit::Meters, "Circle radius");
    colliderFields(circle);

    auto polygon = types.component<PolygonColliderComponent>();
    polygon.describe("A convex polygon shape (3 to 8 vertices) on the entity's physics body.")
        .field("vertexCount", &PolygonColliderComponent::vertexCount, Unit::None,
               "How many vertices are used, 3 to 8")
        .field("vertex0", &PolygonColliderComponent::vertex0, Unit::Meters, "Vertex 1")
        .field("vertex1", &PolygonColliderComponent::vertex1, Unit::Meters, "Vertex 2")
        .field("vertex2", &PolygonColliderComponent::vertex2, Unit::Meters, "Vertex 3")
        .field("vertex3", &PolygonColliderComponent::vertex3, Unit::Meters, "Vertex 4")
        .field("vertex4", &PolygonColliderComponent::vertex4, Unit::Meters, "Vertex 5")
        .field("vertex5", &PolygonColliderComponent::vertex5, Unit::Meters, "Vertex 6")
        .field("vertex6", &PolygonColliderComponent::vertex6, Unit::Meters, "Vertex 7")
        .field("vertex7", &PolygonColliderComponent::vertex7, Unit::Meters, "Vertex 8");
    colliderFields(polygon);
}

} // namespace engine
