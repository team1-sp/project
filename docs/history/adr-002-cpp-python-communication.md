# ADR-002 — C++ to Python Communication

- **Date:** 2026-11-04
- **Status:** Accepted on 2026-11-04 by Işık Dönger (DEC-038)
- **Context:** PB-065 requires a C++ engine to Python agent bridge for headless RL playtesting and
  later assistant interactions. The engine must stay independent from upper AI layers and support
  multiple simultaneous game instances.
- **Options considered:** gRPC/protobuf, ZeroMQ/protobuf, shared memory, raw sockets and in-process
  Python bindings, including cppzmq/pyzmq with MessagePack or FlatBuffers. Detailed evidence is in
  the [IPC option matrix](../research/ipc.md) and
  [PB-065 benchmark report](../research/cpp-python-bridge.md).
- **Decision:** Use gRPC with protobuf for the first fault-isolated production bridge. Keep
  an optional pybind11 Python extension as a local RL fast path if end-to-end profiling justifies it.
  Core engine and RL logic depend only on the lightweight observation/action interface. Keep all
  communication implementations optional and out of the default engine build.
- **Rationale:** Both candidates met the latency needs in the controlled benchmark. ZeroMQ scaled
  better at 4–8 clients, but gRPC supplies typed RPCs, deadlines, status semantics and mature tooling.
  FlatBuffers was slower than protobuf on the same ZeroMQ topology. pybind11 was orders of magnitude
  faster in-process, especially with a 64 KiB buffer view, but gives up isolation and remote use.
- **Consequences:** The first bridge task will need pinned gRPC/protobuf dependencies and generated
  code. ZeroMQ/FlatBuffers remains benchmark evidence, not a production dependency. A pybind11 RL
  adapter should expose vectorized steps and buffers rather than many tiny threaded calls. The seam
  prevents core simulation or RL rewrites if evidence later changes the transport.
- **Revisit when:** end-to-end RL measurements show bridge overhead is material; batching or streaming
  requirements are known; more than 8 concurrent local games are needed; or Windows packaging makes
  gRPC disproportionately costly.
- **Windows evidence (2026-11-04):** the bridge builds and passes with MSVC 14.44 and vcpkg after four
  build fixes (PRB-004); the first vcpkg install took about 22 minutes. On that machine ZeroMQ had
  3.4–5.5x gRPC's throughput at 8 clients, a wider gap than on macOS
  ([results](../research/cpp-python-bridge.md)). gRPC stayed under 1.7 ms p99, so the decision stands.
- **Related:** PB-065, DEC-005, DEC-011, DEC-013, DEC-019, DEC-029, DEC-038.

The repository's binding decision source is `docs/history/decision-log.md`; DEC-038 records this ADR as
Accepted. A later change of transport needs a new decision entry that supersedes DEC-038.
