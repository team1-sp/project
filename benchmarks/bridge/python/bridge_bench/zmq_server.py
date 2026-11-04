import argparse
import signal
import threading

import zmq

from . import agent_bridge_pb2
from .responder import respond


def worker(context: zmq.Context, endpoint: str) -> None:
    socket = context.socket(zmq.REP)
    socket.linger = 0
    socket.connect(endpoint)
    try:
        while True:
            encoded = socket.recv()
            request = agent_bridge_pb2.Observation.FromString(encoded)
            socket.send(respond(request).SerializeToString())
    except zmq.ContextTerminated:
        pass
    finally:
        socket.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--bind", default="tcp://127.0.0.1:5555")
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
    backend_endpoint = "inproc://bridge-benchmark-workers"
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
    print(f"READY zeromq {args.bind} workers={args.workers}", flush=True)
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
