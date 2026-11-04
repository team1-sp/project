import argparse
import signal
import threading

import flatbuffers
import zmq

from .flat_generated.Aegis.Bridge.Benchmark import Action, Observation


def respond(encoded: bytes) -> bytes:
    request = Observation.Observation.GetRootAs(encoded)
    builder = flatbuffers.Builder(64)
    Action.ActionStart(builder)
    Action.ActionAddGameInstanceId(builder, request.GameInstanceId())
    Action.ActionAddSequence(builder, request.Sequence())
    Action.ActionAddActionId(builder, 7)
    action = Action.ActionEnd(builder)
    builder.Finish(action)
    return bytes(builder.Output())


def worker(context: zmq.Context, endpoint: str) -> None:
    socket = context.socket(zmq.REP)
    socket.linger = 0
    socket.connect(endpoint)
    try:
        while True:
            socket.send(respond(socket.recv()))
    except zmq.ContextTerminated:
        pass
    finally:
        socket.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--bind", default="tcp://127.0.0.1:5556")
    parser.add_argument("--workers", type=int, default=8)
    args = parser.parse_args()
    if args.workers < 1:
        parser.error("--workers must be positive")

    context = zmq.Context()
    frontend = context.socket(zmq.ROUTER)
    backend = context.socket(zmq.DEALER)
    frontend.linger = 0
    backend.linger = 0
    frontend.bind(args.bind)
    backend_endpoint = "inproc://bridge-flatbuffers-workers"
    backend.bind(backend_endpoint)
    threads = [
        threading.Thread(target=worker, args=(context, backend_endpoint), daemon=True)
        for _ in range(args.workers)
    ]
    for thread in threads:
        thread.start()

    def stop(_signum, _frame) -> None:
        context.destroy(linger=0)

    signal.signal(signal.SIGTERM, stop)
    signal.signal(signal.SIGINT, stop)
    print(f"READY zeromq-flatbuffers {args.bind} workers={args.workers}", flush=True)
    try:
        zmq.proxy(frontend, backend)
    except zmq.ContextTerminated:
        pass
    finally:
        frontend.close()
        backend.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
