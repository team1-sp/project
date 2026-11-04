# Aegis AI Services

Python package for Aegis's AI layer. This skeleton provides the shared project tooling only;
RL, bug detection, assistant and narrative capabilities will be added by their own tasks.

## Requirements

- Python 3.12 (the project rejects other minor versions)
- `pip`

## Setup

From `ai-services/`, create an isolated environment and install the package with its pinned
development tools.

macOS:

```sh
python3.12 -m venv .venv
.venv/bin/python -m pip install --upgrade pip
.venv/bin/python -m pip install -e ".[dev]"
```

Windows PowerShell:

```powershell
py -3.12 -m venv .venv
.venv\Scripts\python -m pip install --upgrade pip
.venv\Scripts\python -m pip install -e ".[dev]"
```

## Checks

Run from `ai-services/` with the virtual environment activated, or replace `python` with the
environment-specific path shown above:

```sh
python -m ruff check .
python -m pytest
```
