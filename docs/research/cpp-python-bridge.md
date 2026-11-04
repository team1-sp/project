# C++ to Python Communication Research (PB-065)

**Status:** benchmarked on 2026-11-04; recommendation is provisional pending team approval and
end-to-end RL workload evidence. Raw results: [pb-065-benchmark-results.json](pb-065-benchmark-results.json).
Decision proposal: [ADR-002](../history/adr-002-cpp-python-communication.md). The required IPC option
matrix, including cppzmq/MessagePack, FlatBuffers and Windows effort, is in [ipc.md](ipc.md).

## 1. Existing architecture

The engine is C++20 (DEC-013/019), must run headless (DEC-005), and must not depend on the upper
Semantic or AI layers (DEC-001). The repository layout already reserves `engine/bridge/`, `proto/`
and Python `ai-services/`. The Automated Playtest Agent needs an action interface; the assistant is
read-only toward engine state and has a much looser 30-second response target. DEC-011 previously
proposed Python plus gRPC/protobuf but was deliberately left undecided.

The engine should therefore own observation/action values and call a narrow synchronous bridge
interface. Transport implementations may depend on gRPC or ZeroMQ, but simulation, ECS and RL logic
must not. This is enough isolation to revisit the transport without creating a generic messaging
framework.

## 2. Options

| Criterion | gRPC + protobuf | ZeroMQ + protobuf |
| --- | --- | --- |
| Latency/throughput | Sub-millisecond median at low concurrency here; throughput flattened around 4 clients | Sub-millisecond median; scaled better from 2–8 clients here |
| Serialization | Generated protobuf is integrated; framework adds HTTP/2/RPC machinery | Same protobuf cost, plus explicit encode/decode in application code |
| Synchronous request/reply | Native unary RPC, deadlines and status codes | REQ/REP supports it; timeout recovery and socket state need explicit care |
| Concurrent games | HTTP/2 multiplexing and server thread pool | ROUTER/DEALER broker and worker pool are explicit and flexible |
| C++/Python integration | More build/code-generation work; strong generated contract | Small C API/client; server topology and protocol conventions are application-owned |
| Maintain/debug | Reflection/tooling, named methods, typed errors and mature observability | Easy wire topology inspection, but fewer built-in RPC semantics |
| Portability/deploy | Larger dependency graph (HTTP/2, TLS, protobuf, codegen) | Smaller transport dependency, but protobuf/codegen remains for fair typed messages |
| RL training | Suitable for control-plane and step-by-step remote environments; batching/streaming can be added | Attractive for high-rate local multi-environment stepping and custom routing |
| Assistant | Natural named APIs, deadlines and future streaming | Works, but adds protocol conventions with little assistant-specific benefit |

### Alternatives

- **Shared memory:** potentially lowest local-copy overhead, but requires synchronization, framing,
  crash recovery and platform-specific lifecycle work. Reconsider only if profiling shows the bridge
  is a training bottleneck after batching/vectorized environments.
- **Raw sockets:** save framework overhead but recreate framing, schemas, timeouts, concurrency,
  error semantics and debugging. Not justified for this project stage.
- **Python bindings/in-process embedding:** pybind11 or nanobind can remove IPC and may suit a Gym-like
  training API. They couple Python ABI/lifetime/GIL concerns to the engine process and provide fault
  isolation poorly. Reconsider for a dedicated local-training executable, not as the only assistant
  and playtest boundary.
- **Files/stdin/JSON:** useful for offline telemetry and manual debugging, but inefficient and weakly
  typed for repeated step/action exchange.

## 3. Benchmark design

The gRPC and ZeroMQ/protobuf C++ clients use the same protobuf `Observation` and `Action`. The
ZeroMQ/FlatBuffers client uses an equivalent schema. The observation contains a game
instance ID, sequence and byte payload; the Python responder preserves both IDs and returns action 7.
There is no simulation or inference. The C++ client uses a monotonic high-resolution clock around
the complete request/reply call. Incomplete, timed-out, malformed or mis-associated replies are not
counted as throughput.

The baseline used release binaries, TCP loopback, 8 Python worker threads and 2-second timeouts.
IPC cases used 1,000 warm-ups and 10,000 measured exchanges **per client**; faster pybind11 cases
used 10,000 warm-ups and 100,000 measurements. Client counts were 1, 2, 4 and 8;
payloads were 256 bytes and 65,536 bytes. gRPC used a `ThreadPoolExecutor(max_workers=8)`. Both
ZeroMQ variants used the same ROUTER/DEALER proxy plus 8 REP workers; only protobuf versus
FlatBuffers changed. The pybind11 extension ran in-process with Python threads and measured copied
input versus direct byte-buffer views. It is a performance ceiling, not fault-isolated IPC.

Machine: MacBook Pro Mac14,9, Apple M2 Pro (10 cores), 32 GB RAM, macOS 26.6.2 arm64; Apple Clang
17.0.0; CMake 4.4.4; Python 3.12.13; gRPC C++/Python 1.85.1; protobuf 7.36.2; FlatBuffers
25.12.19; cppzmq 4.11.0; pybind11 3.1.0; libzmq 4.3.5_2; pyzmq 27.2.0. Full setup and commands are in
[the benchmark README](../../benchmarks/bridge/README.md).

## 4. Results

All 6,900,000 measured exchanges completed successfully; errors/timeouts: 0. The complete 40-case
median/p95/p99 dataset is in [the raw JSON](pb-065-benchmark-results.json).

### 256-byte observation

| Configuration | Exchanges/s at 1 / 2 / 4 / 8 clients | Median µs at 1 / 8 | 8-client p95 / p99 µs |
| --- | ---: | ---: | ---: |
| gRPC + protobuf | 6,043 / 8,076 / 8,215 / 8,093 | 161 / 979 | 1,239 / 1,408 |
| ZeroMQ + protobuf | 4,839 / 8,604 / 13,530 / 20,461 | 203 / 390 | 458 / 496 |
| ZeroMQ + FlatBuffers | 4,475 / 8,108 / 12,827 / 18,004 | 218 / 436 | 604 / 690 |
| pybind11 copy | 2,244,805 / 1,218,789 / 305,974 / 164,481 | 0.292 / 3.959 | 301 / 648 |
| pybind11 buffer view | 2,500,649 / 1,105,223 / 328,395 / 164,569 | 0.250 / 3.875 | 301 / 657 |

### 65,536-byte observation

| Configuration | Exchanges/s at 1 / 2 / 4 / 8 clients | Median µs at 1 / 8 | 8-client p95 / p99 µs |
| --- | ---: | ---: | ---: |
| gRPC + protobuf | 5,354 / 7,442 / 7,704 / 7,509 | 181 / 1,057 | 1,326 / 1,492 |
| ZeroMQ + protobuf | 4,130 / 7,113 / 10,826 / 15,363 | 237 / 510 | 643 / 741 |
| ZeroMQ + FlatBuffers | 3,908 / 6,579 / 10,411 / 14,888 | 249 / 525 | 714 / 807 |
| pybind11 copy | 493,384 / 333,503 / 236,065 / 222,145 | 1.916 / 2.833 | 183 / 534 |
| pybind11 buffer view | 2,512,226 / 1,199,566 / 344,619 / 163,846 | 0.250 / 4.000 | 300 / 642 |

FlatBuffers did not improve the ZeroMQ path: protobuf was faster in all tested cases, so another
wire format is not justified by this payload shape. pybind11 reduced single-client call overhead by
roughly two orders of magnitude; the 64 KiB buffer view was about 5.1 times faster than copying.
Python-thread throughput and tail latency degraded as concurrency increased, suggesting a future RL
binding should expose vectorized C++ stepping rather than many tiny threaded calls. These empty-work
calls do not predict complete simulation or inference throughput.

## 5. Recommendation

Provisionally choose **gRPC + protobuf** for the first fault-isolated production bridge, behind
`AgentBridge`. Add **pybind11 as an optional Python-driven local RL adapter only when an end-to-end
training profile justifies it**. The binding should batch/vectorize environments and prefer buffer
views. Do not adopt FlatBuffers now; it added complexity and was slower than protobuf here.

Revisit if an end-to-end vectorized RL benchmark shows bridge overhead is material, if more than
8 simultaneous local instances are required, or if deployment/build cost blocks gRPC on Windows.
At that point compare async gRPC, batching/streaming, ZeroMQ and the measured pybind11 path using
actual simulation observations and inference.

## 6. PB-065 acceptance and subtask map

| Item | Evidence | Status |
| --- | --- | --- |
| Acceptance: options researched | gRPC/ZeroMQ comparison and alternative analysis in section 2 | Implemented |
| Acceptance: two options benchmarked | Five configurations share one 40-case matrix; raw results linked above | Implemented and run on macOS |
| Acceptance: decision recorded as ADR | ADR-002 and DEC-038 propose gRPC behind `AgentBridge` | Implemented, awaiting human acceptance |
| PB-065.1 research communication options | Sections 1–2 cover required criteria and alternatives | Complete |
| PB-065.2 gRPC ping-pong | C++ client, concurrent Python server, statistics, timeout/error handling | Implemented and tested |
| PB-065.3 ZeroMQ ping-pong | C++ client, concurrent ROUTER/DEALER Python server, same protobuf/statistics | Implemented and tested |
| Follow-up: ZeroMQ + FlatBuffers | Equivalent schema and topology, generated C++/Python bindings | Implemented, tested and measured |
| Follow-up: pybind11 | In-process copy and buffer-view extension baseline | Implemented, tested and measured |
| PB-065.4 record ADR-002 | `docs/history/adr-002-cpp-python-communication.md` plus DEC-038 | Proposed, not binding |
| Multiple-client correctness | Automated 1- and 4-client integration tests for all three IPC configurations and both pybind11 modes | Tested |
| Portability | Default engine build has no new transport dependency; optional Ubuntu CI job and platform setup are documented | Implemented; Ubuntu CI and Windows benchmark not run in this environment |

Remaining work is production transport implementation, end-to-end simulation/inference benchmarking,
Windows benchmark execution, and a human decision on ADR-002. Those are deliberately outside this
research spike; no production gRPC or ZeroMQ dependency has been introduced.

## 7. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-04 | PB-065 implementation, controlled benchmark and provisional recommendation | Codex |
