#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "aegis/assistant/chat_store.hpp"
#include "aegis/assistant/grounded/citations.hpp"
#include "aegis/assistant/grounded/content_hash.hpp"
#include "grounded_fakes.hpp"

namespace {

using namespace aegis::assistant;

GroundedCitation roundTrip(const GroundedCitation& citation) {
    const Citation encoded = encodeCitation(citation);
    const DecodedCitation decoded = decodeCitation(encoded);
    EXPECT_TRUE(decoded.succeeded()) << decoded.error;
    return decoded.citation.value_or(GroundedCitation{});
}

TEST(GroundedCitation, StateAtTick_RoundTripsWithKindState) {
    const GroundedCitation citation = StateCitation{3, StatePoint::atTick(41)};
    const Citation encoded = encodeCitation(citation);
    EXPECT_EQ(encoded.kind, "state");
    EXPECT_EQ(encoded.payload, "run=3\npoint=tick:41");
    EXPECT_EQ(roundTrip(citation), citation);
}

TEST(GroundedCitation, StateAtStart_IsDistinctFromTickZero) {
    const GroundedCitation start = StateCitation{3, StatePoint::start()};
    const GroundedCitation tickZero = StateCitation{3, StatePoint::atTick(0)};
    EXPECT_NE(start, tickZero);
    EXPECT_EQ(encodeCitation(start).payload, "run=3\npoint=start");
    EXPECT_EQ(encodeCitation(tickZero).payload, "run=3\npoint=tick:0");
    EXPECT_EQ(roundTrip(start), start);
    EXPECT_EQ(roundTrip(tickZero), tickZero);
    EXPECT_TRUE(std::get<StateCitation>(roundTrip(start)).point.isStart());
    EXPECT_EQ(std::get<StateCitation>(roundTrip(tickZero)).point.tick(), engine::Tick{0});
    EXPECT_EQ(describeCitation(start), "run 3, Start");
    EXPECT_EQ(describeCitation(tickZero), "run 3, tick 0");
}

TEST(GroundedCitation, EntityVariants_RoundTripAllFields) {
    const EntityFacts door{engine::EntityId{2}, "Door = A\nB%", "Level1", false};
    const EntityFacts runtime{engine::EntityId{(std::uint64_t{1} << 32) + 5}, "Bullet", "Level1",
                              true};
    const GroundedCitation current = EntityCitation{door, std::nullopt};
    const GroundedCitation inRun = EntityCitation{runtime, RunReference{4, std::nullopt}};
    const GroundedCitation atStart = EntityCitation{door, RunReference{4, StatePoint::start()}};
    const GroundedCitation atTick = EntityCitation{runtime, RunReference{4, StatePoint::atTick(9)}};

    EXPECT_EQ(encodeCitation(current).kind, "entity");
    for (const GroundedCitation& citation : {current, inRun, atStart, atTick}) {
        EXPECT_EQ(roundTrip(citation), citation);
    }
    EXPECT_TRUE(std::get<EntityCitation>(roundTrip(inRun)).entity.createdAtRuntime);
    EXPECT_FALSE(std::get<EntityCitation>(roundTrip(inRun)).run->point.has_value());
}

TEST(GroundedCitation, Source_RoundTripsFileLineAndHash) {
    const GroundedCitation citation =
        SourceCitation{"Scripts/door_system.cpp", 12, contentHash("int main() {}\n")};
    const Citation encoded = encodeCitation(citation);
    EXPECT_EQ(encoded.kind, "source");
    EXPECT_EQ(roundTrip(citation), citation);
    EXPECT_EQ(describeCitation(citation), "Scripts/door_system.cpp:12");
}

TEST(GroundedCitation, MalformedPayloads_AreRejectedNotGuessed) {
    EXPECT_FALSE(decodeCitation("state", "run=3").succeeded());
    EXPECT_FALSE(decodeCitation("state", "point=start\nrun=3").succeeded());
    EXPECT_FALSE(decodeCitation("state", "run=3\npoint=tick 41").succeeded());
    EXPECT_FALSE(decodeCitation("state", "run=03\npoint=start").succeeded());
    EXPECT_FALSE(decodeCitation("state", "run=-1\npoint=start").succeeded());
    EXPECT_FALSE(decodeCitation("state", "run=3\npoint=start\nextra=1").succeeded());
    EXPECT_FALSE(decodeCitation("state", "The door failed at tick 41").succeeded());
    EXPECT_FALSE(decodeCitation("entity", "id=0\nname=x\nscene=s\nruntime=false").succeeded());
    EXPECT_FALSE(decodeCitation("entity", "id=2\nname=x\nscene=s\nruntime=yes").succeeded());
    EXPECT_FALSE(decodeCitation("source", "file=a\nline=0\nhash=h").succeeded());
    EXPECT_FALSE(decodeCitation("source", "file=a\nline=1\nhash=").succeeded());
    EXPECT_FALSE(decodeCitation("state", "run=3\npoint=%zz").succeeded());
    EXPECT_FALSE(decodeCitation("tick", "run=3\npoint=start").succeeded());
    EXPECT_FALSE(decodeCitation("state", "").succeeded());
}

TEST(GroundedCitation, TypedCitations_SurviveChatStoreRoundTrip) {
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() /
        ("aegis-us111-citations-" +
         std::to_string(::testing::UnitTest::GetInstance()->random_seed()));
    std::filesystem::remove_all(root);
    const grounded_test::FixedUserProjectPaths paths(root);
    const ChatStore store(paths);

    const std::vector<GroundedCitation> citations = {
        StateCitation{1, StatePoint::start()},
        StateCitation{1, StatePoint::atTick(0)},
        EntityCitation{{engine::EntityId{2}, "Door", "Level1", false},
                       RunReference{1, StatePoint::atTick(5)}},
        SourceCitation{"Scripts/a.cpp", 3, contentHash("a\nb\nc\n")},
    };
    Chat chat;
    chat.id = "citations";
    ChatMessage answer;
    answer.role = MessageRole::Answer;
    for (const GroundedCitation& citation : citations) {
        answer.citations.push_back(encodeCitation(citation));
    }
    chat.messages.push_back(answer);
    ASSERT_TRUE(store.save(chat).succeeded());

    const LoadResult loaded = store.load("citations");
    ASSERT_TRUE(loaded.succeeded()) << loaded.error;
    ASSERT_EQ(loaded.chat->messages.at(0).citations.size(), citations.size());
    for (std::size_t index = 0; index < citations.size(); ++index) {
        const DecodedCitation decoded = decodeCitation(loaded.chat->messages[0].citations[index]);
        ASSERT_TRUE(decoded.succeeded()) << decoded.error;
        EXPECT_EQ(*decoded.citation, citations[index]);
    }
    std::filesystem::remove_all(root);
}

// FIPS 180-4 test vectors.
TEST(ContentHash, KnownVectors_MatchSha256) {
    EXPECT_EQ(contentHash(""),
              "sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    EXPECT_EQ(contentHash("abc"),
              "sha256:ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    EXPECT_EQ(contentHash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
              "sha256:248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    EXPECT_NE(contentHash("int x = 1;\n"), contentHash("int x = 2;\n"));
}

} // namespace
