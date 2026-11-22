#include "aegis/assistant/grounded/citations.hpp"

#include <charconv>
#include <cstddef>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace aegis::assistant {
namespace {

constexpr std::string_view startValue = "start";
constexpr std::string_view tickPrefix = "tick:";
constexpr std::size_t maximumPayloadBytes = 64U * 1024U;

// Only '%', '\n' and '\r' are escaped: lines split records and the first '=' splits a key.
[[nodiscard]] std::string escapeValue(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char character : value) {
        switch (character) {
        case '%':
            escaped += "%25";
            break;
        case '\n':
            escaped += "%0A";
            break;
        case '\r':
            escaped += "%0D";
            break;
        default:
            escaped += character;
            break;
        }
    }
    return escaped;
}

[[nodiscard]] std::optional<std::string> unescapeValue(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] != '%') {
            result += value[index];
            continue;
        }
        if (value.size() - index < 3U) {
            return std::nullopt;
        }
        const std::string_view code = value.substr(index + 1, 2);
        if (code == "25") {
            result += '%';
        } else if (code == "0A") {
            result += '\n';
        } else if (code == "0D") {
            result += '\r';
        } else {
            return std::nullopt;
        }
        index += 2;
    }
    return result;
}

template <typename Integer>
[[nodiscard]] std::optional<Integer> parseUnsigned(std::string_view text) {
    static_assert(std::is_unsigned_v<Integer>);
    if (text.empty() || (text.size() > 1 && text.front() == '0')) {
        return std::nullopt;
    }
    Integer value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
        return std::nullopt;
    }
    return value;
}

[[nodiscard]] std::string encodePoint(StatePoint point) {
    if (point.isStart()) {
        return std::string(startValue);
    }
    return std::string(tickPrefix) + std::to_string(*point.tick());
}

[[nodiscard]] std::optional<StatePoint> decodePoint(std::string_view text) {
    if (text == startValue) {
        return StatePoint::start();
    }
    if (!text.starts_with(tickPrefix)) {
        return std::nullopt;
    }
    const auto tick = parseUnsigned<engine::Tick>(text.substr(tickPrefix.size()));
    if (!tick) {
        return std::nullopt;
    }
    return StatePoint::atTick(*tick);
}

using Field = std::pair<std::string, std::string>;

[[nodiscard]] std::string joinFields(const std::vector<Field>& fields) {
    std::string payload;
    for (const Field& field : fields) {
        if (!payload.empty()) {
            payload += '\n';
        }
        payload += field.first;
        payload += '=';
        payload += escapeValue(field.second);
    }
    return payload;
}

// Reads exactly the expected keys, in order. Optional keys may be absent but not reordered.
class FieldReader {
public:
    explicit FieldReader(std::string_view payload) {
        std::size_t begin = 0;
        while (begin <= payload.size()) {
            const std::size_t end = payload.find('\n', begin);
            const std::string_view line =
                payload.substr(begin, end == std::string_view::npos ? end : end - begin);
            const std::size_t separator = line.find('=');
            if (separator == std::string_view::npos || separator == 0) {
                valid_ = false;
                return;
            }
            const std::optional<std::string> value = unescapeValue(line.substr(separator + 1));
            if (!value) {
                valid_ = false;
                return;
            }
            fields_.emplace_back(std::string(line.substr(0, separator)), *value);
            if (end == std::string_view::npos) {
                break;
            }
            begin = end + 1;
        }
    }

    [[nodiscard]] bool valid() const noexcept { return valid_; }

    [[nodiscard]] std::optional<std::string> required(std::string_view key) {
        if (!valid_ || next_ >= fields_.size() || fields_[next_].first != key) {
            valid_ = false;
            error_ = "missing or misplaced key: " + std::string(key);
            return std::nullopt;
        }
        return fields_[next_++].second;
    }

    [[nodiscard]] std::optional<std::string> optional(std::string_view key) {
        if (!valid_ || next_ >= fields_.size() || fields_[next_].first != key) {
            return std::nullopt;
        }
        return fields_[next_++].second;
    }

    [[nodiscard]] bool finished() const noexcept { return valid_ && next_ == fields_.size(); }
    [[nodiscard]] const std::string& error() const noexcept { return error_; }

private:
    std::vector<Field> fields_;
    std::size_t next_ = 0;
    bool valid_ = true;
    std::string error_;
};

[[nodiscard]] DecodedCitation failure(std::string reason) {
    return {std::nullopt, std::move(reason)};
}

[[nodiscard]] DecodedCitation decodeState(FieldReader& reader) {
    const auto run = reader.required("run");
    const auto point = reader.required("point");
    if (!run || !point || !reader.finished()) {
        return failure("invalid state citation fields");
    }
    const auto runNumber = parseUnsigned<std::uint32_t>(*run);
    const auto statePoint = decodePoint(*point);
    if (!runNumber || !statePoint) {
        return failure("invalid state citation run or point");
    }
    return {StateCitation{*runNumber, *statePoint}, {}};
}

[[nodiscard]] DecodedCitation decodeEntity(FieldReader& reader) {
    const auto id = reader.required("id");
    const auto name = reader.required("name");
    const auto scene = reader.required("scene");
    const auto runtime = reader.required("runtime");
    const auto run = reader.optional("run");
    const auto point = run ? reader.optional("point") : std::nullopt;
    if (!id || !name || !scene || !runtime || !reader.finished()) {
        return failure("invalid entity citation fields");
    }
    const auto entityId = parseUnsigned<std::uint64_t>(*id);
    if (!entityId || *entityId == 0 || (*runtime != "true" && *runtime != "false")) {
        return failure("invalid entity citation id or runtime flag");
    }
    EntityCitation citation{
        EntityFacts{engine::EntityId{*entityId}, *name, *scene, *runtime == "true"}, {}};
    if (run) {
        const auto runNumber = parseUnsigned<std::uint32_t>(*run);
        if (!runNumber) {
            return failure("invalid entity citation run");
        }
        RunReference reference{*runNumber, std::nullopt};
        if (point) {
            reference.point = decodePoint(*point);
            if (!reference.point) {
                return failure("invalid entity citation point");
            }
        }
        citation.run = reference;
    }
    return {std::move(citation), {}};
}

[[nodiscard]] DecodedCitation decodeSource(FieldReader& reader) {
    const auto file = reader.required("file");
    const auto line = reader.required("line");
    const auto hash = reader.required("hash");
    if (!file || !line || !hash || !reader.finished()) {
        return failure("invalid source citation fields");
    }
    const auto lineNumber = parseUnsigned<std::uint32_t>(*line);
    if (file->empty() || !lineNumber || *lineNumber == 0 || hash->empty()) {
        return failure("invalid source citation file, line or hash");
    }
    return {SourceCitation{*file, *lineNumber, *hash}, {}};
}

} // namespace

Citation encodeCitation(const GroundedCitation& citation) {
    if (const auto* state = std::get_if<StateCitation>(&citation)) {
        return {std::string(stateCitationKind),
                joinFields({{"run", std::to_string(state->runNumber)},
                            {"point", encodePoint(state->point)}})};
    }
    if (const auto* entity = std::get_if<EntityCitation>(&citation)) {
        std::vector<Field> fields{
            {"id", std::to_string(static_cast<std::uint64_t>(entity->entity.id))},
            {"name", entity->entity.name},
            {"scene", entity->entity.sourceScene},
            {"runtime", entity->entity.createdAtRuntime ? "true" : "false"}};
        if (entity->run) {
            fields.emplace_back("run", std::to_string(entity->run->runNumber));
            if (entity->run->point) {
                fields.emplace_back("point", encodePoint(*entity->run->point));
            }
        }
        return {std::string(entityCitationKind), joinFields(fields)};
    }
    const auto& source = std::get<SourceCitation>(citation);
    return {std::string(sourceCitationKind), joinFields({{"file", source.file},
                                                         {"line", std::to_string(source.line)},
                                                         {"hash", source.contentHash}})};
}

DecodedCitation decodeCitation(std::string_view kind, std::string_view payload) {
    if (payload.empty() || payload.size() > maximumPayloadBytes) {
        return failure("citation payload is empty or too large");
    }
    FieldReader reader(payload);
    if (!reader.valid()) {
        return failure("citation payload is not key=value lines");
    }
    if (kind == stateCitationKind) {
        return decodeState(reader);
    }
    if (kind == entityCitationKind) {
        return decodeEntity(reader);
    }
    if (kind == sourceCitationKind) {
        return decodeSource(reader);
    }
    return failure("unknown citation kind: " + std::string(kind));
}

DecodedCitation decodeCitation(const Citation& citation) {
    return decodeCitation(citation.kind, citation.payload);
}

std::string describeStatePoint(std::uint32_t runNumber, StatePoint point) {
    std::string text = "run " + std::to_string(runNumber) + ", ";
    if (point.isStart()) {
        return text + "Start";
    }
    return text + "tick " + std::to_string(*point.tick());
}

std::string describeCitation(const GroundedCitation& citation) {
    if (const auto* state = std::get_if<StateCitation>(&citation)) {
        return describeStatePoint(state->runNumber, state->point);
    }
    if (const auto* entity = std::get_if<EntityCitation>(&citation)) {
        std::string text = "entity " +
                           std::to_string(static_cast<std::uint64_t>(entity->entity.id)) + " \"" +
                           entity->entity.name + "\"";
        if (entity->run) {
            text += entity->run->point
                        ? " at " + describeStatePoint(entity->run->runNumber, *entity->run->point)
                        : " in run " + std::to_string(entity->run->runNumber);
        }
        return text;
    }
    const auto& source = std::get<SourceCitation>(citation);
    return source.file + ':' + std::to_string(source.line);
}

} // namespace aegis::assistant
