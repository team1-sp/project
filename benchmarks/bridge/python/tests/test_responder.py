from bridge_bench import agent_bridge_pb2
from bridge_bench.responder import respond


def test_respond_preserves_client_and_sequence() -> None:
    request = agent_bridge_pb2.Observation(
        game_instance_id=23, sequence=91, payload=b"observation"
    )

    action = respond(request)

    assert action.game_instance_id == 23
    assert action.sequence == 91
    assert action.action_id == 7
