# C++ to Python Communication Research (PB-065)

**Status:** benchmarked on macOS and Windows on 2026-11-04; recommendation accepted the same day (DEC-038). Revisit with
end-to-end RL workload evidence. Raw results: [pb-065-benchmark-results.json](pb-065-benchmark-results.json).
Decision: [ADR-002](../history/adr-002-cpp-python-communication.md). The required IPC option
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

### Windows run

Run on 2026-11-04 to check the Windows-priority platform (DEC-029). Machine: HP OMEN MAX 16 laptop,
Intel Core Ultra 9 275HX (24 cores, hybrid P/E), 63.4 GB RAM, Windows 11 Home 10.0.26200, on AC power
with the Balanced power plan; MSVC 14.44 (VS 2022), CMake 4.4.3, Python 3.12.0; vcpkg baseline
`482b8b1756b3` with gRPC 1.84.0, protobuf 7.36.2, ZeroMQ 4.3.5, cppzmq 4.11.0, FlatBuffers 25.12.19;
Python grpcio 1.85.1, pyzmq 27.2.0, pybind11 3.1.0. Same matrix and settings as above.

The first vcpkg install took 22.2 minutes. The build needed four fixes, now on the branch (PRB-004);
after them the build with warnings as errors, CTest (4/4) and all 40 cases passed with 0 errors.

| Configuration | 256 B: exchanges/s at 1 / 2 / 4 / 8 | 256 B: median µs 1 / 8 | 64 KiB: exchanges/s at 1 / 2 / 4 / 8 | 64 KiB: median µs 1 / 8 |
| --- | ---: | ---: | ---: | ---: |
| gRPC + protobuf | 3,641 / 4,759 / 6,557 / 7,109 | 270 / 1,118 | 3,339 / 5,314 / 6,069 / 6,017 | 290 / 1,319 |
| ZeroMQ + protobuf | 7,433 / 13,404 / 25,519 / 39,417 | 130 / 200 | 5,325 / 11,023 / 16,696 / 20,627 | 173 / 370 |
| ZeroMQ + FlatBuffers | 5,680 / 13,167 / 24,031 / 33,668 | 147 / 224 | 5,440 / 11,070 / 15,988 / 20,184 | 169 / 381 |
| pybind11 copy | 1,148,574 / 1,059,093 / 351,834 / 342,462 | 0.7 / 20.6 | 389,438 / 271,255 / 212,138 / 213,463 | 2.3 / 5.8 |
| pybind11 buffer view | 1,338,921 / 1,188,499 / 353,985 / 376,192 | 0.6 / 2.3 | 937,007 / 547,986 / 314,253 / 307,607 | 0.9 / 25.0 |

8-client p95 / p99 (µs), 256 B: gRPC 1,313 / 1,432; ZeroMQ 244 / 279; ZeroMQ + FlatBuffers 339 / 411.
64 KiB: gRPC 1,542 / 1,678; ZeroMQ 526 / 633; ZeroMQ + FlatBuffers 548 / 671.

On Windows the gap is wider than on macOS: ZeroMQ was about 2x gRPC's throughput at 1 client and
3.4–5.5x at 8 clients, and gRPC's median passed 1 ms at 8 clients. gRPC still stays under 1.7 ms p99
at 8 clients, so it remains usable for step-by-step exchange at these rates. FlatBuffers and protobuf
were within 15% on ZeroMQ, with FlatBuffers slightly ahead in two 64 KiB cases, so the macOS finding
that protobuf won every case does not hold on Windows; neither result justifies a second wire format.

Limits: one pass with no repeats on a laptop with hybrid cores and a Balanced power plan, so small
gaps are noise. The C++ client used gRPC 1.84.0 against a Python grpcio 1.85.1 server. Do not compare
these numbers directly with the macOS table. The raw Windows JSON is kept on the test machine and was
not committed; its metadata fields for compiler and native libraries are placeholders on Windows, so
the versions above are the record.

## 5. Recommendation

Choose **gRPC + protobuf** for the first fault-isolated production bridge, behind
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
| Acceptance: two options benchmarked | Five configurations share one 40-case matrix; raw results linked above | Implemented and run on macOS and Windows |
| Acceptance: decision recorded as ADR | ADR-002 and DEC-038 choose gRPC behind `AgentBridge` | Accepted by Işık Dönger, 2026-11-04 |
| PB-065.1 research communication options | Sections 1–2 cover required criteria and alternatives | Complete |
| PB-065.2 gRPC ping-pong | C++ client, concurrent Python server, statistics, timeout/error handling | Implemented and tested |
| PB-065.3 ZeroMQ ping-pong | C++ client, concurrent ROUTER/DEALER Python server, same protobuf/statistics | Implemented and tested |
| Follow-up: ZeroMQ + FlatBuffers | Equivalent schema and topology, generated C++/Python bindings | Implemented, tested and measured |
| Follow-up: pybind11 | In-process copy and buffer-view extension baseline | Implemented, tested and measured |
| PB-065.4 record ADR-002 | `docs/history/adr-002-cpp-python-communication.md` plus DEC-038 | Accepted |
| Multiple-client correctness | Automated 1- and 4-client integration tests for all three IPC configurations and both pybind11 modes | Tested |
| Portability | Default engine build has no new transport dependency; optional Ubuntu CI job and platform setup are documented | Implemented; Ubuntu `bridge-benchmark` CI passes after the PRB-002 fix; Windows builds and passes after the PRB-004 fixes and was benchmarked |

Remaining work is production transport implementation and end-to-end simulation/inference
benchmarking. Those are deliberately outside this
research spike; no production gRPC or ZeroMQ dependency has been introduced.

## 7. Change table

| Date | Change | By |
| --- | --- | --- |
| 2026-11-04 | PB-065 implementation, controlled benchmark and provisional recommendation | Codex |
| 2026-11-04 | Recommendation accepted (DEC-038) | claude (PR #23 review) |
| 2026-11-04 | Windows build fixes and Windows benchmark results added | claude (PR #23 review) |
