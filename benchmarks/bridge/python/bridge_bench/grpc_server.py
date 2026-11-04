import argparse
import concurrent.futures
import signal
import sys

import grpc

from . import agent_bridge_pb2_grpc
from .responder import respond


class AgentService(agent_bridge_pb2_grpc.AgentServiceServicer):
    def Exchange(self, request, context):
        return respond(request)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--bind", default="127.0.0.1:50051")
    parser.add_argument("--workers", type=int, default=8)
    args = parser.parse_args()
    if args.workers < 1:
        parser.error("--workers must be positive")

    server = grpc.server(
        concurrent.futures.ThreadPoolExecutor(max_workers=args.workers)
    )
    agent_bridge_pb2_grpc.add_AgentServiceServicer_to_server(AgentService(), server)
    if server.add_insecure_port(args.bind) == 0:
        print(f"failed to bind {args.bind}", file=sys.stderr)
        return 2
    server.start()
    print(f"READY grpc {args.bind} workers={args.workers}", flush=True)

    def stop(_signum, _frame) -> None:
        server.stop(grace=1)

    signal.signal(signal.SIGTERM, stop)
    signal.signal(signal.SIGINT, stop)
    server.wait_for_termination()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
