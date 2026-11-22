"""Label open pull requests that conflict with main, and unlabel them once they don't (DEC-055).

Runs from .github/workflows/conflict-flag.yml after every push to main (another PR was merged)
and on every push to a PR branch. A PR that now conflicts gets the `needs-sync` label and one
comment asking its author to merge main into the branch; the label goes away when the conflict
is resolved. GitHub computes mergeability lazily, so UNKNOWN is retried a few times.
Rules: docs/guidelines/git-workflow.md (Merge conflicts)
"""
import json
import os
import subprocess
import sys
import time

LABEL = "needs-sync"
QUERY = """
query($owner: String!, $name: String!) {
  repository(owner: $owner, name: $name) {
    pullRequests(states: OPEN, first: 100) {
      nodes { number mergeable isDraft labels(first: 50) { nodes { name } } }
    }
  }
}
"""
COMMENT = (
    "This pull request now conflicts with `main`, probably because another PR was merged. "
    "The author merges `main` into this branch (a merge, not a rebase), resolves the conflicts "
    "by the rules in `docs/guidelines/git-workflow.md` (Merge conflicts), reruns the tests and "
    "pushes. The `needs-sync` label goes away by itself after that push."
)


def gh(*args, body=None):
    result = subprocess.run(["gh", *args], input=body, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"gh {' '.join(args[:2])} failed: {result.stderr.strip()}")
    return result.stdout


def open_prs(owner, name):
    out = gh("api", "graphql", "-f", f"query={QUERY}", "-F", f"owner={owner}", "-F", f"name={name}")
    return json.loads(out)["data"]["repository"]["pullRequests"]["nodes"]


def main():
    owner, name = os.environ["GITHUB_REPOSITORY"].split("/")
    only = os.environ.get("PR_NUMBER")
    prs = []
    for attempt in range(6):
        prs = [p for p in open_prs(owner, name) if not only or str(p["number"]) == only]
        if all(p["mergeable"] != "UNKNOWN" for p in prs):
            break
        time.sleep(10)
    for pr in prs:
        number, state = str(pr["number"]), pr["mergeable"]
        labelled = any(label["name"] == LABEL for label in pr["labels"]["nodes"])
        if state == "CONFLICTING" and not labelled:
            gh("pr", "edit", number, "--add-label", LABEL)
            gh("pr", "comment", number, "--body-file", "-", body=COMMENT)
            print(f"PR #{number}: conflicts with main, labelled {LABEL}")
        elif state == "MERGEABLE" and labelled:
            gh("pr", "edit", number, "--remove-label", LABEL)
            print(f"PR #{number}: no longer conflicts, label removed")
        else:
            print(f"PR #{number}: {state}")


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as error:
        print(f"::error::{error}")
        sys.exit(1)
