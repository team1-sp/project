# C++ to Python Bridge Benchmark

PB-065 compares gRPC/protobuf, ZeroMQ/protobuf, ZeroMQ/FlatBuffers and an in-process pybind11
baseline without game simulation or RL inference. IPC backends use equivalent messages and a
deterministic Python responder. pybind11 measures direct copied-buffer and buffer-view calls.
The C++ clients report successful exchanges only; timeouts, protocol failures and mismatched
client/sequence IDs are errors and produce a non-zero exit code.

## Method

- IPC clients send `game_instance_id`, `sequence` and an observation byte payload to Python.
- Python returns the same IDs and the fixed action `7`; the pybind11 baseline calls C++ directly.
- C++ uses `std::chrono::steady_clock`; reported latency is round-trip latency, not one-way.
- Baseline transport is TCP loopback.
- Every client warms up independently before a shared measured phase.
- gRPC uses a Python `ThreadPoolExecutor`; both ZeroMQ variants use ROUTER/DEALER plus REP workers.
- pybind11 uses Python threads and reports in-process call latency, not network round-trip latency.
- Defaults: 8 server workers, 1/2/4/8 clients, 256 B/64 KiB payloads, and a 2-second IPC timeout.
  IPC uses 1,000 warm-ups + 10,000 measurements per client; faster pybind11 cases use 10,000
  warm-ups + 100,000 measurements so their measured interval is meaningful.

This is a microbenchmark. It does not measure game simulation, tensor conversion, batching,
RL inference, cross-host networking or training throughput.

## Dependencies

- CMake 3.25+, a C++20 compiler and Python 3.12
- protobuf/`protoc`, FlatBuffers/`flatc`, and pybind11
- gRPC C++ and its `grpc_cpp_plugin`
- libzmq
- Python dependencies pinned in `python/pyproject.toml`

macOS with Homebrew:

```sh
brew install cmake cppzmq flatbuffers grpc zeromq
python3.12 -m venv .venv
.venv/bin/python -m pip install -e "benchmarks/bridge/python[dev]"
```

Ubuntu/Debian:

```sh
sudo apt-get install cmake g++ cppzmq-dev flatbuffers-compiler libflatbuffers-dev libgrpc++-dev \
  libprotobuf-dev libzmq3-dev protobuf-compiler protobuf-compiler-grpc python3.12-venv
python3.12 -m venv .venv
.venv/bin/python -m pip install -e "benchmarks/bridge/python[dev]"
```

On Windows, install gRPC, protobuf and ZeroMQ with vcpkg, pass its CMake toolchain file, and
use the Python virtual environment's `Scripts/python.exe` below.

## Build and test

From the repository root:

```sh
cmake -S . -B build/bridge-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DENGINE_WITH_RAYLIB=OFF \
  -DENGINE_BUILD_TESTS=ON \
  -DAEGIS_BUILD_BRIDGE_BENCHMARKS=ON \
  -DAEGIS_BRIDGE_PYTHON_EXECUTABLE="$PWD/.venv/bin/python"
cmake --build build/bridge-release --parallel
ctest --test-dir build/bridge-release --output-on-failure
```

The integration test starts each Python server and verifies one and four concurrent C++ clients.
It requires permission to bind loopback ports.

## Run the reproducible matrix

```sh
PYTHONPATH=benchmarks/bridge/python \
  .venv/bin/python benchmarks/bridge/python/run_benchmarks.py \
  --build-dir build/bridge-release/benchmarks/bridge \
  --output docs/research/pb-065-benchmark-results.json
```

The JSON report records the configuration, environment, errors, throughput, median latency, p95 and
p99. The pybind11 numbers are an in-process ceiling and must not be labelled RTT. Do not compare
results collected on different machines as if they were one controlled experiment. Run the full
matrix on the same idle machine and commit raw output rather than copying selected values.
