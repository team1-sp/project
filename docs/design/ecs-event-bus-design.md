# PB-010.2 (companion) — Event Bus Design


## 1. What the SRS fixes

| Rule | SRS |
| --- | --- |
| Systems publish typed events and subscribe to the types they handle | 2.45 |
| Each event gets a sequence number (publication order) and its publication tick, is queued, and is dispatched **at the end of the tick phase in which it was published** | 2.46 |
| Delivery: events in sequence order; each event to its subscribers in **system run order**; several callbacks of one system in subscription order | 2.46 |
| Every event type has a semantic description that declares its payload fields | 2.47, 2.84 |
| Handlers finish before the next phase starts, so later phases see their changes | 2.77 |
| An event published by a handler during a dispatch waits for the **next** dispatch point and keeps its publication tick; after the last phase that is the end of the first phase of the next tick | 2.78 |
| A subscriber receives only events published after it subscribed | 2.79 |
| At most **10,000 dispatched events per tick** (deferred events count first); after the limit every event reaching a dispatch point that tick is dropped with reason "limit"; one error per tick names the type with most drops | 2.80 |
| Telemetry records each event once at publication, then its final status | 2.81 |
| States: published → queued → dispatched \| dropped; removed from the queue before delivery | 2.83 |
| At run end, queued events are dropped with reason "run ended" and written to telemetry before the file closes | 2.102 |
| A disabled system or group receives nothing; an event whose subscribers are all disabled is still "dispatched" | 2.116 |
| Subscribing or unsubscribing during a dispatch takes effect after that dispatch point | 2.117 |
| Events are delivered even if an entity they name is gone; entities whose destruction was requested in this phase are still found by this phase's handlers | 2.118, 2.119 |

## 2. Design choices

| # | Choice | Why |
| --- | --- | --- |
| E-1 | Subscribers are **systems only**. A handler is a member function of the subscribing system and runs with **that system's `SystemContext`** (same access rules, same phase `CommandBuffer`) | 2.46 and 2.116 speak of systems; keeps the access check (D-11) complete |
| E-2 | Engine modules (Physics Adapter) may **publish** but not subscribe; telemetry watches through an **observer**, not a subscription | Telemetry must see every event, including dropped ones (2.81) |
| E-3 | `publish<E>` and `subscribe<E>` must be declared in `describe()` (`publishes<…>`, `subscribes<…>`); violations stop the run like other access violations | Extends D-11; gives the Semantic Graph `publishes`/`handles` edges |
| E-4 | Event payloads use the component reflection and the same managed/unmanaged rule (D-14). Entity fields are `EntityId` (D-2) | One reflection system; 2.84 needs list payloads |
| E-5 | Storage: one typed buffer per event type plus one ordered queue of `{seq, type index, slot}` | No per-event heap allocation after warm-up for unmanaged events (coding rule 7) |

E-1…E-5 were not asked separately; they follow from the SRS and from D-2, D-11 and D-14. Review them in PB-010.3.

## 3. Event types

```cpp
struct CollisionEvent {                       // published by the Physics Adapter
    static constexpr std::string_view typeName = "Collision";
    EntityId a = noEntity;
    EntityId b = noEntity;
    CollisionKind kind = CollisionKind::Begin;   // registered enum: Begin, End
};

struct ItemCollectedEvent {
    static constexpr std::string_view typeName = "ItemCollected";
    EntityId collector = noEntity;            // the Player
    EntityId item = noEntity;                 // the Collectible entity
    AssetId itemId{};                         // srs.fr.2.84
};

struct UnlockFailedEvent {                    // managed: holds a copy of the Inventory
    static constexpr std::string_view typeName = "UnlockFailed";
    EntityId player = noEntity;
    EntityId unlockable = noEntity;
    AssetId requiredItem{};
    std::vector<AssetId> inventory;           // captured from the state the decision used (2.84)
};

void registerGameplayEvents(TypeRegistry& types) {
    types.event<UnlockFailedEvent>()
        .describe("A player tried to unlock an Unlockable without holding its required item.")
        .field("player", &UnlockFailedEvent::player, Unit::None, "Entity that tried")
        .field("unlockable", &UnlockFailedEvent::unlockable, Unit::None, "Entity that stayed locked")
        .field("requiredItem", &UnlockFailedEvent::requiredItem, Unit::None, "Item it needs")
        .field("inventory", &UnlockFailedEvent::inventory, Unit::None, "Player's items at the attempt");
}
```

The struct is the **payload**; the bus adds the envelope:

```cpp
using Tick = std::uint64_t;                   // engine/core; tick numbers start at 0
enum class EventSeq : std::uint64_t {};       // per run, starts at 1, publication order

struct EventInfo {
    EventSeq seq;
    Tick publishedTick;                       // 2.46, 2.47
    TickPhase publishedPhase;
    TypeId type;
    PublisherId publisher;                    // system name or engine module, e.g. "PhysicsAdapter"
};

enum class EventStatus : std::uint8_t { Queued, Dispatched, Dropped };
enum class DropReason : std::uint8_t { None, Limit, RunEnded };
```

## 4. Publishing and subscribing

```cpp
class UnlockSystem final : public ISystem {
public:
    void describe(SystemDescription& d) const override {
        d.name("Unlock")
            .purpose("Decides whether a player who touches an Unlockable unlocks it.")
            .phase(TickPhase::GameplayRules)
            .reads<InventoryComponent, PlayerControlledTag>()
            .writes<UnlockableComponent>()
            .subscribes<CollisionEvent>()
            .publishes<UnlockedEvent, UnlockFailedEvent>();
    }

    void setup(SystemContext& ctx) override {
        ctx.subscribe<CollisionEvent>(&UnlockSystem::onCollision);
    }

    void update(SystemContext&) override {}   // all work happens in the handler

private:
    void onCollision(const EventInfo& info, const CollisionEvent& hit, SystemContext& ctx) {
        // Runs at the dispatch point of the phase that published the event (PhysicsSyncOut).
        // Structural changes go to ctx.commands() and apply at the end of that phase.
        // ...decide using the Inventory, then:
        ctx.publish(UnlockFailedEvent{/* ... */});   // waits for the next dispatch point (2.78)
    }
};
```

- `ctx.publish(event)` returns the `EventSeq`, stamps the current tick and phase, and notifies the observers immediately (2.81).
- `ctx.subscribe<E>(&S::handler)` binds to the calling system's instance; `S` must be that system's class. It returns a `SubscriptionId`; `ctx.unsubscribe(id)` removes it. Both may be called at any time; during a dispatch they take effect after it (2.117).
- A handler runs at the dispatch point of the phase **where the event was published**, not in the subscriber's own phase. Its timing is added to the subscriber's per-tick time (srs.fr.2.17).
- Engine modules publish directly on the bus: `world.events().publish(PublisherId::engine("PhysicsAdapter"), CollisionEvent{…})`.

## 5. Bus API

```cpp
class EventBus {
public:
    static constexpr std::size_t maxDispatchedPerTick = 10'000;   // srs.fr.2.80

    template <EventType E> EventSeq publish(PublisherId from, E event);
    template <EventType E> SubscriptionId subscribe(SystemId system, Handler<E> handler);
    void unsubscribe(SubscriptionId id);

    void beginTick(Tick tick);          // resets the per-tick limit counters
    void dispatch(TickPhase phase);     // end of every phase, before command playback
    void endTick();                     // logs the one "limit" error for this tick, if any
    void endRun();                      // drops everything still queued (2.102)

    void addObserver(IEventObserver& observer);
};

class IEventObserver {                  // telemetry; engine-internal
public:
    virtual ~IEventObserver() = default;
    virtual void onPublished(const EventInfo& info, const void* payload) = 0;
    virtual void onFinalized(EventSeq seq, EventStatus status, Tick tick, DropReason reason) = 0;
};
```

`Handler<E>` is `std::function<void(const EventInfo&, const E&, SystemContext&)>`; `SystemContext::subscribe` builds it from the member-function pointer.

## 6. Dispatch algorithm

State: `queue_` (events waiting for the next dispatch point, ascending seq), `deferred_` (published by handlers during the current dispatch), `dispatchedThisTick_`, `limitReached_`, `dropsByType_`.

```
dispatch(phase):                          // after the phase's systems, before command playback
    batch = take(queue_)                  // only events published before this point
    freeze subscriber lists               // changes during the loop wait (2.117)
    for ev in batch, ascending seq:
        if limitReached_ or dispatchedThisTick_ == maxDispatchedPerTick:
            limitReached_ = true
            finalize(ev, Dropped, Limit); dropsByType_[ev.type] += 1
            continue                      // not delivered, not carried over (2.80)
        dispatchedThisTick_ += 1          // ev is already out of the queue (2.83)
        for sub in subscribers(ev.type):  // system run order, then subscription order (2.46)
            if sub's system or group is disabled: continue           // 2.116
            if sub.subscribedAfterSeq >= ev.seq: continue            // 2.79
            sub.handler(ev.info, payload(ev), contextOf(sub.system))
        finalize(ev, Dispatched, currentTick)                         // even with no receiver
    queue_ = take(deferred_)              // handler-published events: next dispatch point (2.78)
    apply pending subscribe/unsubscribe
```

- `subscribedAfterSeq` is the last sequence number published when the subscription was made.
- Events published by **systems** during `update()` go straight to `queue_` and are dispatched at the end of the same phase. Events published by **handlers** go to `deferred_`.
- The queue survives the end of a tick: events deferred during the last phase's dispatch reach the first dispatch point of the next tick, carry their old publication tick, and have the lowest sequence numbers, so they count toward the next tick's limit first (2.78, 2.80).
- `finalize` updates the status and calls `IEventObserver::onFinalized`.

## 7. Place in the tick

```
phase N:  systems update()  →  events.dispatch(N)  →  commands.playback()  →  phase N+1
                                 handlers run here      destroys and adds apply here
```

- Handlers of phase N still find entities whose destruction was requested in phase N, because destruction happens at playback, after dispatch (2.118, 2.119). Requests made by handlers in `PhysicsSyncOut` also apply at the end of that phase, so every CollisionEvent of the step is delivered first (2.120).
- An event that names an entity destroyed at an **earlier** phase boundary is still delivered with its `EntityId`; `ctx.find(id)` returns nothing for it (2.118).
- The Physics Adapter publishes no CollisionEvent for a body whose entity was destroyed before collision collection (2.118, srs.fr.7.21). That rule belongs to the adapter design.

## 8. Run end

`World::endRun()` calls `EventBus::endRun()`: every event in `queue_` and `deferred_` is finalized as **Dropped / RunEnded**, the observers are notified, and only then may telemetry close the file (2.102). This happens for every end reason: max ticks, objective reached, failure, stopped, error, window closed.

## 9. Telemetry records (2.81)

The telemetry observer writes one record at publication and one when the status is final. Entity IDs are found through the reflection (`EntityRef` fields), so the `entities` list needs no extra code per event type.

```json
{"type":"event","seq":118,"tick":412,"phase":"PhysicsSyncOut","name":"Collision","publisher":"PhysicsAdapter","entities":[4294967301,4294967310],"payload":{"a":4294967301,"b":4294967310,"kind":"Begin"}}
{"type":"eventStatus","seq":118,"status":"dispatched","tick":412}
{"type":"eventStatus","seq":131,"status":"dropped","reason":"limit"}
```

The record shapes follow the `[PROPOSED]` format in `telemetry-and-events.md` and are settled with the telemetry PB item. The schema record (main document §12) lists each event type with its fields, and each system's `publishes` and `subscribes`. The Semantic Graph gets `System —publishes→ Event` and `System —handles→ Event` edges from it.

## 10. Implementation notes

- **Typed buffers** (E-5): `std::vector<E>` per event type, reused between ticks; an entry is released when its event is finalized. The global queue holds `{seq, typeIndex, slot}` in seq order, so dispatch is a linear walk.
- **Subscriber lists** per event type are kept sorted by (system run index, subscription order) and re-sorted when the scheduler's order changes.
- The bus is single-threaded, like the rest of the simulation.

## 11. Test checklist

Sequence numbers increase in publication order · dispatch at the end of the publishing phase · handler-published event waits for the next dispatch point and keeps its tick, also across the tick boundary · delivery in system run order and subscription order · subscriber gets only later events · disabled system or group receives nothing and the event is still "dispatched" · subscribe/unsubscribe during dispatch applies afterwards · limit: 10,000th dispatched, 10,001st dropped "limit", later events that tick dropped, deferred events counted first next tick, one error per tick · run end drops queued and deferred with "run ended" before telemetry closes · handler finds an entity whose destruction was requested in the same phase · undeclared publish or subscribe stops the run.
