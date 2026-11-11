"""Pull request hygiene checks: branch name and linked issue.

Reads the pull_request event from GITHUB_EVENT_PATH. Rules: docs/guidelines/git-workflow.md
"""
import json
import os
import re
import sys

# <type>/<issue number>-<short-description>, or <issue number>-<short-description> as GitHub's
# "Create a branch" button names it. The old ClickUp form <type>/pb-<nnn>-... is still accepted
# so branches started before the move to GitHub Projects can merge.
# Keep in sync with the branch name rule of the GitHub ruleset, if one is set.
BRANCH = re.compile(
    r"^((feat|fix|test|refactor|docs|ci|chore|research)/(\d+|pb-\d{3}(\.\d+)?)|\d+)"
    r"-[a-z0-9]+(-[a-z0-9]+)*$"
)
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
            f"Branch '{branch}' does not match <type>/<issue>-<short-description> or "
            "<issue>-<short-description>, all lowercase (for example "
            "feat/41-run-every-simulation-tick). "
            "Types: feat, fix, test, refactor, docs, ci, chore, research."
        )
    if not CLOSES.search(body) and "no-issue" not in labels:
        errors.append(
            "The PR body has no 'Closes #<issue>'. Link the issue, or add the "
            "no-issue label for a chore/ci PR without one."
        )

    for error in errors:
        print(f"::error::{error}")
    if not errors:
        print(f"Branch '{branch}' and issue link look good.")
    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()
