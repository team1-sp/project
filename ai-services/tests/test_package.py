from importlib.metadata import version

import aegis_ai_services


def test_package_version_matches_installed_metadata() -> None:
    assert aegis_ai_services.__version__ == version("aegis-ai-services")
