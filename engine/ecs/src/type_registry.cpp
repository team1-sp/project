#include "engine/ecs/type_registry.hpp"

#include <algorithm>

#include "engine/ecs/built_in_components.hpp"
#include "engine/ecs/core_components.hpp"

namespace engine {

namespace {

bool idLess(const std::pair<TypeId, std::uint32_t>& entry, TypeId id) {
    return entry.first < id;
}

} // namespace

TypeRegistry::TypeRegistry() {
    // The built-in components have no enabled flag: the entity's own flag covers them (DEC-47.1).
    component<NameComponent>()
        .describe("The entity's name, shown in the Hierarchy and the Inspector.")
        .alwaysEnabled()
        .field("name", &NameComponent::name, Unit::None,
               "Name of the entity, UTF-8, at most 64 bytes");
    component<EntityInfoComponent>()
        .describe("The entity's tag and layer and the scene it belongs to.")
        .alwaysEnabled()
        .field("tag", &EntityInfoComponent::tag, Unit::None, "One value of the project's tag list")
        .field("layer", &EntityInfoComponent::layer, Unit::None, "Layer from 0 to 31; 0 is Default")
        .field("sourceScene", &EntityInfoComponent::sourceScene, Unit::None,
               "Scene the entity was loaded from or created in");
    component<HierarchyComponent>()
        .describe("The entity's parent and its children in order, as a list of siblings.")
        .alwaysEnabled()
        .field("parent", &HierarchyComponent::parent, Unit::None, "Parent entity; none for a root")
        .field("firstChild", &HierarchyComponent::firstChild, Unit::None, "First child entity")
        .field("nextSibling", &HierarchyComponent::nextSibling, Unit::None,
               "Next child of the same parent")
        .field("prevSibling", &HierarchyComponent::prevSibling, Unit::None,
               "Previous child of the same parent")
        .field("childCount", &HierarchyComponent::childCount, Unit::None, "Number of children");
    for (ComponentTypeInfo& info : components_) {
        info.builtIn = true;
    }
}

void TypeRegistry::finalize() {
    checkOpen("TypeRegistry::finalize");
    std::vector<std::string> problems = problems_;
    std::vector<std::string_view> names;
    for (std::size_t i = 0; i < components_.size(); ++i) {
        const ComponentTypeInfo& info = components_[i];
        names.push_back(info.name);
        if (info.description.empty()) {
            problems.push_back("component " + info.name + " has no description");
        }
        for (std::size_t f = 0; f < info.fields.size(); ++f) {
            const FieldInfo& field = info.fields[f];
            if (field.description.empty()) {
                problems.push_back("field " + info.name + "." + field.name + " has no description");
            }
            for (std::size_t g = 0; g < f; ++g) {
                if (info.fields[g].name == field.name) {
                    problems.push_back("component " + info.name + " has two fields named " +
                                       field.name);
                }
            }
        }
        if (info.fields.size() != cppTypes_[i].memberCount) {
            problems.push_back("component " + info.name + " has " +
                               std::to_string(cppTypes_[i].memberCount) + " data members but " +
                               std::to_string(info.fields.size()) + " registered fields");
        }
        for (std::size_t j = 0; j < i; ++j) {
            if (components_[j].id == info.id && components_[j].name != info.name) {
                problems.push_back("components " + components_[j].name + " and " + info.name +
                                   " have the same name hash");
            }
        }
    }
    for (const EnumInfo& info : enums_) {
        names.push_back(info.name);
        if (info.description.empty()) {
            problems.push_back("enum " + info.name + " has no description");
        }
        for (const EnumValueInfo& value : info.values) {
            if (value.description.empty()) {
                problems.push_back("enum value " + info.name + "." + value.name +
                                   " has no description");
            }
        }
    }
    std::sort(names.begin(), names.end());
    for (std::size_t i = 1; i < names.size(); ++i) {
        if (names[i] == names[i - 1]) {
            problems.push_back("two types are named " + std::string(names[i]));
        }
    }
    if (!problems.empty()) {
        std::string message = "TypeRegistry::finalize:";
        for (const std::string& problem : problems) {
            message += " " + problem + ";";
        }
        throw RunError(message);
    }

    byId_.clear();
    for (const ComponentTypeInfo& info : components_) {
        byId_.emplace_back(info.id, info.index);
    }
    std::sort(byId_.begin(), byId_.end());
    finalized_ = true;
}

const ComponentTypeInfo* TypeRegistry::find(TypeId id) const {
    if (finalized_) {
        const auto at = std::lower_bound(byId_.begin(), byId_.end(), id, idLess);
        return at != byId_.end() && at->first == id ? &components_[at->second] : nullptr;
    }
    for (const ComponentTypeInfo& info : components_) {
        if (info.id == id) {
            return &info;
        }
    }
    return nullptr;
}

const ComponentTypeInfo* TypeRegistry::find(std::string_view name) const {
    const ComponentTypeInfo* info = find(typeIdFromName(name));
    return info != nullptr && info->name == name ? info : nullptr;
}

const EnumInfo* TypeRegistry::findEnum(std::string_view name) const {
    for (const EnumInfo& info : enums_) {
        if (info.name == name) {
            return &info;
        }
    }
    return nullptr;
}

std::unique_ptr<IComponentPool> TypeRegistry::makePool(const ComponentTypeInfo& info) const {
    return cppTypes_[info.index].makePool(!info.alwaysEnabled);
}

void TypeRegistry::checkOpen(std::string_view operation) const {
    if (finalized_) {
        throw RunError(std::string(operation) + ": the type registry is already finalized");
    }
}

const std::string* TypeRegistry::enumNameOf(const std::type_info& type) const {
    for (std::size_t i = 0; i < enumTypes_.size(); ++i) {
        if (enumTypes_[i] == std::type_index(type)) {
            return &enums_[i].name;
        }
    }
    return nullptr;
}

void registerCoreComponents(TypeRegistry& types) {
    types.component<PositionComponent>()
        .describe("Where the entity is in the world: position, rotation and scale (meters, y up).")
        .alwaysEnabled()
        .field("x", &PositionComponent::xMeters, Unit::Meters, "Horizontal position")
        .field("y", &PositionComponent::yMeters, Unit::Meters, "Vertical position, up is positive")
        .field("rotation", &PositionComponent::rotationRadians, Unit::Radians,
               "Rotation, counter-clockwise")
        .field("scaleX", &PositionComponent::scaleX, Unit::None, "Horizontal scale factor")
        .field("scaleY", &PositionComponent::scaleY, Unit::None, "Vertical scale factor");
    types.component<VelocityComponent>()
        .describe("How fast the entity moves; the physics body owns it for dynamic bodies.")
        .alwaysEnabled()
        .field("x", &VelocityComponent::xMetersPerSecond, Unit::MetersPerSecond,
               "Horizontal velocity")
        .field("y", &VelocityComponent::yMetersPerSecond, Unit::MetersPerSecond,
               "Vertical velocity, up is positive");
}

} // namespace engine
