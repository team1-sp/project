import grpc

from . import agent_bridge_pb2 as agent__bridge__pb2


class AgentServiceStub:
    def __init__(self, channel: grpc.Channel) -> None:
        self.Exchange = channel.unary_unary(
            "/aegis.bridge.benchmark.AgentService/Exchange",
            request_serializer=agent__bridge__pb2.Observation.SerializeToString,
            response_deserializer=agent__bridge__pb2.Action.FromString,
            _registered_method=True,
        )


class AgentServiceServicer:
    def Exchange(self, request, context):
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details("Method not implemented")
        raise NotImplementedError("Exchange")


def add_AgentServiceServicer_to_server(servicer, server) -> None:
    rpc_method_handlers = {
        "Exchange": grpc.unary_unary_rpc_method_handler(
            servicer.Exchange,
            request_deserializer=agent__bridge__pb2.Observation.FromString,
            response_serializer=agent__bridge__pb2.Action.SerializeToString,
        ),
    }
    generic_handler = grpc.method_handlers_generic_handler(
        "aegis.bridge.benchmark.AgentService", rpc_method_handlers
    )
    server.add_generic_rpc_handlers((generic_handler,))
    server.add_registered_method_handlers(
        "aegis.bridge.benchmark.AgentService", rpc_method_handlers
    )
