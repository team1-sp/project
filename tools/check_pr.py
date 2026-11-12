"""Pull request hygiene checks: branch name and linked issue.

Reads the pull_request event from GITHUB_EVENT_PATH. Rules: docs/guidelines/git-workflow.md
"""
import json
import os
import re
import sys

from issue_ref import BRANCH, may_skip_issue  # noqa: E402  (same folder; rules in issue_ref.py)
EXEMPT_PREFIXES = ("dependabot/",)
CLOSES = re.compile(r"\b(close[sd]?|fix(e[sd])?|resolve[sd]?) #\d+", re.IGNORECASE)


def main():
    with open(os.environ["GITHUB_EVENT_PATH"], encoding="utf-8") as f:
        pr = json.load(f)["pull_request"]
    branch = pr["head"]["ref"]
    body = pr.get("body") or ""
    labels = {label["name"] for label in pr.get("labels", [])}
    errors = []

    if not branch.startswith(EXEMPT_PREFIXES) and not BRANCH.match(branch):
        errors.append(
            f"Branch '{branch}' does not match <type>/us-<x.y>-<short-description> for a "
            "user story (feat/us-15.2-build-settings-file-menu) or "
            "<type>/<issue>-<short-description> for other issues (fix/142-door-opens), "
            "or docs|ci|chore/<short-description> for work without an issue "
            "(ci/update-agent-workflow), all lowercase. "
            "Types: feat, fix, test, refactor, docs, ci, chore, research."
        )
    if not CLOSES.search(body) and "no-issue" not in labels and not may_skip_issue(branch):
        errors.append(
            "The PR body has no 'Closes #<issue>'. Link the issue; only docs, ci and chore "
            "branches (or a PR with the no-issue label) may go without one."
        )

    for error in errors:
        print(f"::error::{error}")
    if not errors:
        print(f"Branch '{branch}' and issue link look good.")
    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()
