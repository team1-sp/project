"""Create or update repository labels from .github/labels.yml.

Usage: GH_TOKEN=... python tools/sync_labels.py owner/repo [--prune]
--prune deletes labels that are not in the file.
"""
import json
import subprocess
import sys
import urllib.parse

import yaml


def gh(*args, body=None):
    cmd = ["gh", "api", *args]
    if body is not None:
        cmd += ["--input", "-"]
    result = subprocess.run(cmd, input=json.dumps(body) if body else None,
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip())
    return json.loads(result.stdout) if result.stdout.strip() else None


def main():
    repo = sys.argv[1]
    prune = "--prune" in sys.argv
    with open(".github/labels.yml", encoding="utf-8") as f:
        wanted = {label["name"]: label for label in yaml.safe_load(f)}
    existing = {label["name"]: label
                for label in gh(f"repos/{repo}/labels?per_page=100", "--paginate")}
    for name, label in wanted.items():
        body = {"new_name": name, "color": label["color"],
                "description": label.get("description", "")}
        if name in existing:
            gh("-X", "PATCH", f"repos/{repo}/labels/{urllib.parse.quote(name)}", body=body)
        else:
            gh("-X", "POST", f"repos/{repo}/labels",
               body={"name": name, "color": label["color"],
                     "description": label.get("description", "")})
        print(f"ok   {name}")
    if prune:
        for name in existing.keys() - wanted.keys():
            gh("-X", "DELETE", f"repos/{repo}/labels/{urllib.parse.quote(name)}")
            print(f"del  {name}")


if __name__ == "__main__":
    main()
