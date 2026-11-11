"""Fail while the pull request's issue still has open "blocked by" issues.

Runs as the `dependencies` job of .github/workflows/ci.yml, which feeds `ci-ok`, so the main
ruleset blocks the merge. The issue is the one the branch names (tools/issue_ref.py), or else
the first `Closes #<n>` in the PR body. After a blocker closes, re-run the job (or push) to
clear it. Rules: docs/guidelines/git-workflow.md
"""
import json
import os
import re
import subprocess
import sys

from issue_ref import find_issue, open_blockers

CLOSES = re.compile(r"\b(?:close[sd]?|fix(?:e[sd])?|resolve[sd]?) #(\d+)", re.IGNORECASE)


def gh(*args):
    result = subprocess.run(["gh", "api", *args], capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"gh api {args[0]} failed: {result.stderr.strip()}")
    return json.loads(result.stdout) if result.stdout.strip() else None


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    with open(os.environ["GITHUB_EVENT_PATH"], encoding="utf-8") as f:
        pr = json.load(f)["pull_request"]
    issue = find_issue(gh, repo, pr["head"]["ref"])
    number = issue["number"] if issue else None
    if number is None:
        match = CLOSES.search(pr.get("body") or "")
        number = int(match.group(1)) if match else None
    if number is None:
        print("No linked issue, so there are no dependencies to check.")
        return 0
    blockers = open_blockers(gh, repo, number)
    for b in blockers:
        print(f"::error::#{number} is blocked by #{b['number']} ({b['title']}), which is still open.")
    if not blockers:
        print(f"#{number} has no open blockers.")
    return 1 if blockers else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except RuntimeError as error:
        print(f"::error::{error}")
        sys.exit(1)
