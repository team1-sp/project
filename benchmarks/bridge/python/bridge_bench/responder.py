from . import agent_bridge_pb2


def respond(request: agent_bridge_pb2.Observation) -> agent_bridge_pb2.Action:
    """Return a deterministic action while preserving request association fields."""
    return agent_bridge_pb2.Action(
        game_instance_id=request.game_instance_id,
        sequence=request.sequence,
        action_id=7,
    )
