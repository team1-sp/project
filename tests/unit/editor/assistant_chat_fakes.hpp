#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "editor/assistant_chat.hpp"
#include "editor/assistant_chat_context.hpp"

// Fakes for the AssistantChat seams. Everything runs on the test thread; no window is opened.
namespace chat_test {

namespace aa = aegis::assistant;

struct AnswerControl {
    bool finished = false;
    int cancelCalls = 0;
    aa::AnswerResult result{aa::MessageState::Complete, {}};
};

class FakeAnswer final : public editor::IActiveAnswer {
public:
    explicit FakeAnswer(std::shared_ptr<AnswerControl> control)
        : control_(std::move(control)) {}

    [[nodiscard]] bool finished() const override { return control_->finished; }
    void cancel() noexcept override { ++control_->cancelCalls; }
    [[nodiscard]] aa::AnswerResult result() const override { return control_->result; }

private:
    std::shared_ptr<AnswerControl> control_;
};

// Appends the question and a Streaming answer like US-11.2, or rejects with `rejection`.
class FakeSender final : public editor::IChatQuestionSender {
public:
    [[nodiscard]] editor::SendOutcome send(aa::Chat& chat,
                                           const aa::GroundedQuestion& question) override {
        questions.push_back(question);
        if (rejection) {
            return {nullptr, *rejection};
        }
        chat.messages.push_back(
            {aa::MessageRole::Question, aa::MessageState::Complete, {}, question.text, {}, {}});
        chat.messages.push_back(
            {aa::MessageRole::Answer, aa::MessageState::Streaming, {}, {}, {}, {}});
        control = std::make_shared<AnswerControl>();
        return {std::make_unique<FakeAnswer>(control), {}};
    }

    std::vector<aa::GroundedQuestion> questions;
    std::optional<std::string> rejection;
    std::shared_ptr<AnswerControl> control;
};

class FakeLoader final : public editor::IChatLoader {
public:
    [[nodiscard]] aa::LoadResult load(const std::string& chatId) const override {
        const auto found = chats.find(chatId);
        if (found == chats.end()) {
            return {std::nullopt, "chat file not found"};
        }
        return {found->second, {}};
    }

    std::map<std::string, aa::Chat> chats;
};

class FakeItems final : public editor::IContextItemProvider {
public:
    [[nodiscard]] std::optional<aa::ContextItem>
    itemFor(editor::ContextChipType type) const override {
        const auto found = items.find(type);
        return found == items.end() ? std::nullopt : std::optional(found->second);
    }

    std::map<editor::ContextChipType, aa::ContextItem> items;
};

// Sizes by type: 100 bytes times (type index + 1).
class FakeSizes final : public editor::IContextSizeEstimator {
public:
    [[nodiscard]] std::optional<std::size_t>
    requestBytes(const aa::ContextItem& item) const override {
        return 100U * (item.index() + 1U);
    }
};

// One item of every US-11.1 context type, in ContextChipType order.
[[nodiscard]] inline std::vector<aa::ContextItem> oneItemOfEveryType() {
    return {
        aa::SelectionContext{{engine::EntityId{7}, engine::EntityId{9}}},
        aa::LastRunContext{},
        aa::GraphContext{},
        aa::GraphNodesContext{{"door", "key", "player"}},
        aa::SourceCodeContext{{"Scripts/door.lua"}},
        aa::AssetContext{"Sprites/door.png"},
        aa::LogEntryContext{aa::LogEntryFacts{"Error", "Door is locked", 41, {}, true}},
        aa::ComponentTypeContext{"Door"},
    };
}

} // namespace chat_test
