import argparse
import importlib.metadata
import json
import os
import platform
import queue
import subprocess
import sys
import threading
import time
from pathlib import Path

TRANSPORTS = {
    "grpc": ("bridge_bench.grpc_server", "grpc_bridge_benchmark", "127.0.0.1:50051"),
    "zeromq": (
        "bridge_bench.zmq_server",
        "zmq_bridge_benchmark",
        "tcp://127.0.0.1:5555",
    ),
    "zeromq-flatbuffers": (
        "bridge_bench.zmq_flatbuffers_server",
        "zmq_flatbuffers_bridge_benchmark",
        "tcp://127.0.0.1:5556",
    ),
}
PYBIND_MODES = ("copy", "view")


def command_version(command: list[str]) -> str:
    try:
        completed = subprocess.run(
            command, check=False, capture_output=True, text=True, timeout=10
        )
        return (completed.stdout or completed.stderr).splitlines()[0]
    except (OSError, subprocess.TimeoutExpired):
        return "unavailable"


def system_metadata() -> dict[str, object]:
    metadata: dict[str, object] = {
        "platform": platform.platform(),
        "machine": platform.machine(),
        "processor": platform.processor() or "unreported",
        "python": platform.python_version(),
        "compiler": command_version([os.environ.get("CXX", "c++"), "--version"]),
        "cmake": command_version(["cmake", "--version"]),
        "libraries": {
            name: importlib.metadata.version(name)
            for name in ("flatbuffers", "grpcio", "protobuf", "pybind11", "pyzmq")
        },
    }
    if platform.system() == "Darwin":
        hardware = subprocess.run(
            ["system_profiler", "SPHardwareDataType"],
            check=False,
            capture_output=True,
            text=True,
            timeout=10,
        ).stdout.splitlines()
        safe_fields = (
            "Model Identifier:",
            "Chip:",
            "Total Number of Cores:",
            "Memory:",
        )
        metadata["hardware"] = "; ".join(
            line.strip() for line in hardware if line.strip().startswith(safe_fields)
        )
        native_versions = subprocess.run(
            [
                "brew",
                "list",
                "--versions",
                "cppzmq",
                "flatbuffers",
                "grpc",
                "protobuf",
                "zeromq",
            ],
            check=False,
            capture_output=True,
            text=True,
            timeout=10,
        ).stdout.splitlines()
        metadata["native_libraries"] = "; ".join(native_versions)
    else:
        metadata["hardware"] = platform.uname().machine
        metadata["native_libraries"] = "record with the platform package manager"
    return metadata


def wait_until_ready(
    server: subprocess.Popen[str], timeout_seconds: float = 10.0
) -> None:
    assert server.stdout is not None
    lines: queue.Queue[str] = queue.Queue(maxsize=1)
    reader = threading.Thread(
        target=lambda: lines.put(server.stdout.readline()), daemon=True
    )
    reader.start()
    try:
        line = lines.get(timeout=timeout_seconds).strip()
    except queue.Empty as exception:
        raise TimeoutError(
            f"server did not become ready within {timeout_seconds:.0f} seconds"
        ) from exception
    if not line.startswith("READY"):
        raise RuntimeError(f"server failed to start: {line or 'no startup output'}")


def run_case(
    build_dir: Path,
    transport: str,
    clients: int,
    payload_bytes: int,
    iterations: int,
    warmup: int,
    workers: int,
    timeout_ms: int,
) -> dict[str, object]:
    module, binary_name, endpoint = TRANSPORTS[transport]
    binary = build_dir / binary_name
    if not binary.is_file():
        raise FileNotFoundError(f"missing benchmark executable: {binary}")
    server = subprocess.Popen(
        [sys.executable, "-m", module, "--bind", endpoint, "--workers", str(workers)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    try:
        wait_until_ready(server)
        completed = subprocess.run(
            [
                str(binary),
                "--endpoint",
                endpoint,
                "--clients",
                str(clients),
                "--iterations",
                str(iterations),
                "--warmup",
                str(warmup),
                "--payload-bytes",
                str(payload_bytes),
                "--timeout-ms",
                str(timeout_ms),
            ],
            check=False,
            capture_output=True,
            text=True,
            timeout=max(60, clients * iterations * timeout_ms // 1000),
        )
        if completed.returncode != 0:
            raise RuntimeError(f"{transport} benchmark failed: {completed.stderr}")
        return json.loads(completed.stdout)
    finally:
        server.terminate()
        try:
            server.wait(timeout=5)
        except subprocess.TimeoutExpired:
            server.kill()
            server.wait(timeout=5)


def run_pybind_case(
    build_dir: Path,
    mode: str,
    clients: int,
    payload_bytes: int,
    iterations: int,
    warmup: int,
    timeout_ms: int,
) -> dict[str, object]:
    environment = os.environ.copy()
    module_dir = str(build_dir / "python")
    environment["PYTHONPATH"] = os.pathsep.join(
        value for value in (module_dir, environment.get("PYTHONPATH")) if value
    )
    completed = subprocess.run(
        [
            sys.executable,
            "-m",
            "bridge_bench.pybind_benchmark",
            "--mode",
            mode,
            "--clients",
            str(clients),
            "--iterations",
            str(iterations),
            "--warmup",
            str(warmup),
            "--payload-bytes",
            str(payload_bytes),
            "--timeout-ms",
            str(timeout_ms),
        ],
        check=False,
        capture_output=True,
        text=True,
        timeout=max(60, clients * iterations * timeout_ms // 1000),
        env=environment,
    )
    if completed.returncode != 0:
        raise RuntimeError(f"pybind11-{mode} benchmark failed: {completed.stderr}")
    return json.loads(completed.stdout)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--clients", type=int, nargs="+", default=[1, 2, 4, 8])
    parser.add_argument("--payload-bytes", type=int, nargs="+", default=[256, 65536])
    parser.add_argument("--iterations", type=int, default=10000)
    parser.add_argument("--warmup", type=int, default=1000)
    parser.add_argument("--pybind-iterations", type=int, default=100000)
    parser.add_argument("--pybind-warmup", type=int, default=10000)
    parser.add_argument("--workers", type=int, default=8)
    parser.add_argument("--timeout-ms", type=int, default=2000)
    parser.add_argument(
        "--pybind-only",
        action="store_true",
        help="replace only pybind11 cases in an existing output report",
    )
    args = parser.parse_args()
    for values in (args.clients, args.payload_bytes):
        if any(value < 1 for value in values):
            parser.error("client counts and payload sizes must be positive")

    report: dict[str, object] = {
        "timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "methodology": {
            "clock": "C++ std::chrono::steady_clock or Python perf_counter_ns (monotonic)",
            "network": "loopback TCP for IPC; in-process for pybind11",
            "latency": "round-trip for IPC; complete call latency for pybind11",
            "server_workers": args.workers,
            "timeout_ms": args.timeout_ms,
            "ipc_iterations_per_client": args.iterations,
            "ipc_warmup_per_client": args.warmup,
            "pybind_iterations_per_client": args.pybind_iterations,
            "pybind_warmup_per_client": args.pybind_warmup,
        },
        "system": system_metadata(),
        "results": [],
    }
    if args.pybind_only:
        if not args.output.is_file():
            parser.error("--pybind-only requires an existing --output report")
        previous = json.loads(args.output.read_text(encoding="utf-8"))
        report["results"] = [
            result
            for result in previous.get("results", [])
            if not str(result.get("transport", "")).startswith("pybind11-")
        ]
    results = report["results"]
    assert isinstance(results, list)
    for payload_bytes in args.payload_bytes:
        for clients in args.clients:
            if not args.pybind_only:
                for transport in TRANSPORTS:
                    result = run_case(
                        args.build_dir,
                        transport,
                        clients,
                        payload_bytes,
                        args.iterations,
                        args.warmup,
                        args.workers,
                        args.timeout_ms,
                    )
                    results.append(result)
                    print(
                        f"{transport} clients={clients} payload={payload_bytes}: "
                        f"{result['throughput_exchanges_per_second']:.1f} exchanges/s"
                    )
            for mode in PYBIND_MODES:
                result = run_pybind_case(
                    args.build_dir,
                    mode,
                    clients,
                    payload_bytes,
                    args.pybind_iterations,
                    args.pybind_warmup,
                    args.timeout_ms,
                )
                results.append(result)
                print(
                    f"pybind11-{mode} clients={clients} payload={payload_bytes}: "
                    f"{result['throughput_exchanges_per_second']:.1f} exchanges/s"
                )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
