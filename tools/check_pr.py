"""Pull request hygiene checks: branch name and linked issue.

Reads the pull_request event from GITHUB_EVENT_PATH. Rules: docs/guidelines/git-workflow.md
"""
import json
import os
import re
import sys

# Any type with a PB id, or `fix/` with a bare issue number (bugs without a PB item).
# Keep in sync with the branch name rule of the GitHub ruleset.
BRANCH = re.compile(
    r"^((feat|fix|test|refactor|docs|ci|chore|research)/pb-\d{3}(\.\d+)?|fix/\d+)"
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
            f"Branch '{branch}' does not match <type>/pb-<nnn>-<short-description> "
            "(or fix/<issue>-<short-description> for a bug without a PB item), all lowercase. "
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
