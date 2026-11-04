import json
import os
import socket
import subprocess
import sys
from pathlib import Path

import pytest


def unused_port() -> int:
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return int(probe.getsockname()[1])


def executable(name: str) -> Path:
    build_dir = os.environ.get("BRIDGE_BENCH_BUILD_DIR")
    if not build_dir:
        pytest.skip("BRIDGE_BENCH_BUILD_DIR is not set")
    candidates = [Path(build_dir) / name, Path(build_dir) / "Debug" / f"{name}.exe"]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    pytest.fail(f"benchmark executable not found: {candidates}")


def benchmark_environment() -> dict[str, str]:
    environment = os.environ.copy()
    pybind_dir = os.environ.get("BRIDGE_BENCH_PYBIND_DIR")
    if not pybind_dir:
        pytest.skip("BRIDGE_BENCH_PYBIND_DIR is not set")
    environment["PYTHONPATH"] = os.pathsep.join(
        value for value in (pybind_dir, environment.get("PYTHONPATH")) if value
    )
    return environment


@pytest.mark.parametrize(
    ("transport", "server_module", "binary_name"),
    [
        ("grpc", "bridge_bench.grpc_server", "grpc_bridge_benchmark"),
        ("zeromq", "bridge_bench.zmq_server", "zmq_bridge_benchmark"),
        (
            "zeromq-flatbuffers",
            "bridge_bench.zmq_flatbuffers_server",
            "zmq_flatbuffers_bridge_benchmark",
        ),
    ],
)
@pytest.mark.parametrize("clients", [1, 4])
def test_request_reply_is_correct_for_concurrent_clients(
    transport: str, server_module: str, binary_name: str, clients: int
) -> None:
    port = unused_port()
    server_endpoint = (
        f"127.0.0.1:{port}" if transport == "grpc" else f"tcp://127.0.0.1:{port}"
    )
    server = subprocess.Popen(
        [
            sys.executable,
            "-m",
            server_module,
            "--bind",
            server_endpoint,
            "--workers",
            "4",
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    try:
        assert server.stdout is not None
        ready = server.stdout.readline().strip()
        assert ready.startswith("READY"), (
            server.stderr.read()
            if server.stderr is not None
            else "server did not become ready"
        )
        completed = subprocess.run(
            [
                str(executable(binary_name)),
                "--endpoint",
                server_endpoint,
                "--clients",
                str(clients),
                "--iterations",
                "25",
                "--warmup",
                "5",
                "--payload-bytes",
                "256",
                "--timeout-ms",
                "2000",
            ],
            check=False,
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert completed.returncode == 0, completed.stderr
        result = json.loads(completed.stdout)
        assert result["errors"] == 0
        assert result["successful_exchanges"] == clients * 25
    finally:
        server.terminate()
        try:
            server.wait(timeout=5)
        except subprocess.TimeoutExpired:
            server.kill()
            server.wait(timeout=5)


@pytest.mark.parametrize(
    ("transport", "binary_name"),
    [
        ("grpc", "grpc_bridge_benchmark"),
        ("zeromq", "zmq_bridge_benchmark"),
        ("zeromq-flatbuffers", "zmq_flatbuffers_bridge_benchmark"),
    ],
)
def test_unavailable_server_reports_failure(transport: str, binary_name: str) -> None:
    port = unused_port()
    endpoint = f"127.0.0.1:{port}" if transport == "grpc" else f"tcp://127.0.0.1:{port}"

    completed = subprocess.run(
        [
            str(executable(binary_name)),
            "--endpoint",
            endpoint,
            "--clients",
            "1",
            "--iterations",
            "1",
            "--warmup",
            "1",
            "--payload-bytes",
            "16",
            "--timeout-ms",
            "50",
        ],
        check=False,
        capture_output=True,
        text=True,
        timeout=10,
    )

    assert completed.returncode != 0
    result = json.loads(completed.stdout)
    assert result["errors"] >= 1
    assert result["successful_exchanges"] == 0


@pytest.mark.parametrize("mode", ["copy", "view"])
@pytest.mark.parametrize("clients", [1, 4])
def test_pybind_request_reply_is_correct(mode: str, clients: int) -> None:
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
            "25",
            "--warmup",
            "5",
            "--payload-bytes",
            "256",
            "--timeout-ms",
            "2000",
        ],
        check=False,
        capture_output=True,
        text=True,
        timeout=30,
        env=benchmark_environment(),
    )
    assert completed.returncode == 0, completed.stderr
    result = json.loads(completed.stdout)
    assert result["errors"] == 0
    assert result["successful_exchanges"] == clients * 25
