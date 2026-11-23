#include "engine/ecs/field.hpp"

#include <cstring>
#include <limits>

namespace engine {

namespace {

template <typename V>
V readAs(const std::byte* at) {
    V value{};
    std::memcpy(&value, at, sizeof(V));
    return value;
}

template <typename V>
void writeAs(std::byte* at, V value) {
    std::memcpy(at, &value, sizeof(V));
}

std::uint64_t readUnsigned(const std::byte* at, std::uint32_t size) {
    switch (size) {
    case 1:
        return readAs<std::uint8_t>(at);
    case 2:
        return readAs<std::uint16_t>(at);
    case 4:
        return readAs<std::uint32_t>(at);
    default:
        return readAs<std::uint64_t>(at);
    }
}

void writeUnsigned(std::byte* at, std::uint32_t size, std::uint64_t value) {
    switch (size) {
    case 1:
        writeAs(at, static_cast<std::uint8_t>(value));
        break;
    case 2:
        writeAs(at, static_cast<std::uint16_t>(value));
        break;
    case 4:
        writeAs(at, static_cast<std::uint32_t>(value));
        break;
    default:
        writeAs(at, value);
        break;
    }
}

std::uint64_t maxOfSize(std::uint32_t size) {
    return size >= 8 ? std::numeric_limits<std::uint64_t>::max()
                     : (std::uint64_t{1} << (8 * size)) - 1;
}

// Copies the value when it holds a V; says what was expected otherwise.
template <typename V>
std::optional<std::string> writeExact(const FieldInfo& field, std::byte* at,
                                      const FieldValue& value) {
    const V* exact = std::get_if<V>(&value);
    if (exact == nullptr) {
        return "field " + field.name + " expects a " + fieldTypeName(field.type) + " value";
    }
    writeAs(at, *exact);
    return std::nullopt;
}

} // namespace

const char* fieldTypeName(FieldType type) {
    switch (type) {
    case FieldType::Bool:
        return "bool";
    case FieldType::Int32:
        return "int32";
    case FieldType::Int64:
        return "int64";
    case FieldType::UInt8:
        return "uint8";
    case FieldType::UInt32:
        return "uint32";
    case FieldType::Float:
        return "float";
    case FieldType::Double:
        return "double";
    case FieldType::Vec2:
        return "vec2";
    case FieldType::Enum:
        return "enum";
    case FieldType::EntityRef:
        return "entityRef";
    case FieldType::AssetRef:
        return "assetRef";
    case FieldType::Tag:
        return "tag";
    case FieldType::FixedString:
        return "fixedString";
    }
    return "unknown";
}

const char* unitName(Unit unit) {
    switch (unit) {
    case Unit::None:
        return "";
    case Unit::Meters:
        return "m";
    case Unit::MetersPerSecond:
        return "m/s";
    case Unit::Radians:
        return "rad";
    case Unit::RadiansPerSecond:
        return "rad/s";
    case Unit::Kilograms:
        return "kg";
    case Unit::Seconds:
        return "s";
    case Unit::Ticks:
        return "ticks";
    }
    return "unknown";
}

FieldValue readField(const FieldInfo& field, const void* component) {
    const std::byte* at = static_cast<const std::byte*>(component) + field.offset;
    switch (field.type) {
    case FieldType::Bool:
        return readAs<bool>(at);
    case FieldType::Int32:
        return readAs<std::int32_t>(at);
    case FieldType::Int64:
        return readAs<std::int64_t>(at);
    case FieldType::UInt8:
        return std::uint32_t{readAs<std::uint8_t>(at)};
    case FieldType::UInt32:
        return readAs<std::uint32_t>(at);
    case FieldType::Float:
        return readAs<float>(at);
    case FieldType::Double:
        return readAs<double>(at);
    case FieldType::Vec2:
        return readAs<Vec2>(at);
    case FieldType::Enum:
        return static_cast<std::int64_t>(readUnsigned(at, field.size));
    case FieldType::EntityRef:
        return readAs<EntityId>(at);
    case FieldType::AssetRef:
        return readAs<AssetId>(at);
    case FieldType::Tag:
        return readAs<TagId>(at);
    case FieldType::FixedString: {
        // FixedString<N> is N chars followed by its length byte (fixed_string.hpp).
        const std::uint8_t length = readAs<std::uint8_t>(at + field.count);
        return std::string(reinterpret_cast<const char*>(at), length);
    }
    }
    return false;
}

std::optional<std::string> writeField(const FieldInfo& field, void* component,
                                      const FieldValue& value) {
    std::byte* at = static_cast<std::byte*>(component) + field.offset;
    switch (field.type) {
    case FieldType::Bool:
        return writeExact<bool>(field, at, value);
    case FieldType::Int32:
        return writeExact<std::int32_t>(field, at, value);
    case FieldType::Int64:
        return writeExact<std::int64_t>(field, at, value);
    case FieldType::UInt8:
    case FieldType::UInt32: {
        const std::uint32_t* number = std::get_if<std::uint32_t>(&value);
        if (number == nullptr) {
            return "field " + field.name + " expects a " + fieldTypeName(field.type) + " value";
        }
        if (*number > maxOfSize(field.size)) {
            return "field " + field.name + " holds at most " +
                   std::to_string(maxOfSize(field.size));
        }
        writeUnsigned(at, field.size, *number);
        return std::nullopt;
    }
    case FieldType::Float:
        return writeExact<float>(field, at, value);
    case FieldType::Double:
        return writeExact<double>(field, at, value);
    case FieldType::Vec2:
        return writeExact<Vec2>(field, at, value);
    case FieldType::Enum: {
        const std::int64_t* number = std::get_if<std::int64_t>(&value);
        if (number == nullptr) {
            return "field " + field.name + " expects an enum value";
        }
        if (*number < 0 || static_cast<std::uint64_t>(*number) > maxOfSize(field.size)) {
            return "field " + field.name + ": " + std::to_string(*number) + " is not a value of " +
                   field.enumType;
        }
        writeUnsigned(at, field.size, static_cast<std::uint64_t>(*number));
        return std::nullopt;
    }
    case FieldType::EntityRef:
        return writeExact<EntityId>(field, at, value);
    case FieldType::AssetRef:
        return writeExact<AssetId>(field, at, value);
    case FieldType::Tag:
        return writeExact<TagId>(field, at, value);
    case FieldType::FixedString: {
        const std::string* text = std::get_if<std::string>(&value);
        if (text == nullptr) {
            return "field " + field.name + " expects a text value";
        }
        if (text->size() > field.count) {
            return "field " + field.name + " holds at most " + std::to_string(field.count) +
                   " bytes";
        }
        std::memset(at, 0, field.count);
        std::memcpy(at, text->data(), text->size());
        writeAs(at + field.count, static_cast<std::uint8_t>(text->size()));
        return std::nullopt;
    }
    }
    return "field " + field.name + " has an unknown type";
}

} // namespace engine
