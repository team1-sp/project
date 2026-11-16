"""Close an epic issue once all of its sub-issues are closed.

Runs from .github/workflows/epic-close.yml. Epics are the issues labelled `type:epic`
(EP-xx); their stories are GitHub sub-issues (DEC-050). Rules: docs/guidelines/git-workflow.md

- ISSUE set (the workflow runs on `issues: closed`): look up the closed issue's parent with the
  sub-issues API. If the parent is an open epic and every one of its sub-issues is closed, close
  it as completed with a comment. Then do the same one level up, because a close made with the
  workflow's own token does not start another workflow run.
- ISSUE empty (run by hand, workflow_dispatch): check every open epic the same way.

An epic with no sub-issues is never closed. The "Later stories" checklist in an epic's body is
not counted: only sub-issues are (a planned story is converted to a sub-issue). If a new story
is added to a closed epic later, reopen the epic by hand.

Env: GH_TOKEN (issues: write), GITHUB_REPOSITORY, ISSUE (optional), DRY_RUN ("true" only prints).
"""
import json
import os
import subprocess
import sys

EPIC_LABEL = "type:epic"
MAX_LEVELS = 5  # parent chain to walk up from one closed issue


def gh(*args, body=None):
    cmd = ["gh", "api", *args]
    if body is not None:
        cmd += ["--input", "-"]
    result = subprocess.run(cmd, input=json.dumps(body) if body is not None else None,
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"gh api {' '.join(args[:3])} failed: {result.stderr.strip()}")
    return json.loads(result.stdout) if result.stdout.strip() else None


def gh_pages(path):
    """All items of a paginated list endpoint."""
    items, page = [], 1
    sep = "&" if "?" in path else "?"
    while True:
        batch = gh(f"{path}{sep}per_page=100&page={page}") or []
        items += batch
        if len(batch) < 100:
            return items
        page += 1


def parent_of(repo, number):
    """The parent issue, or None (the API answers 404 when there is none)."""
    try:
        return gh(f"repos/{repo}/issues/{number}/parent")
    except RuntimeError as e:
        if "404" in str(e) or "Not Found" in str(e):
            return None
        raise


def is_epic(issue):
    return any(label["name"] == EPIC_LABEL for label in issue.get("labels", []))


def try_close(repo, epic, dry_run):
    """Close the epic if it is open and all its sub-issues are closed. True if it closed."""
    number = epic["number"]
    if epic["state"] != "open" or not is_epic(epic):
        print(f"#{number} {epic['title']}: not an open epic, nothing to do.")
        return False
    subs = gh_pages(f"repos/{repo}/issues/{number}/sub_issues")
    if not subs:
        print(f"#{number} {epic['title']}: no sub-issues, left open.")
        return False
    still_open = [s for s in subs if s["state"] == "open"]
    if still_open:
        listed = ", ".join(f"#{s['number']}" for s in still_open)
        print(f"#{number} {epic['title']}: {len(still_open)} of {len(subs)} sub-issues open ({listed}).")
        return False
    if dry_run:
        print(f"#{number} {epic['title']}: all {len(subs)} sub-issues closed, would close (dry run).")
        return True
    gh(f"repos/{repo}/issues/{number}/comments", body={"body": (
        f"All {len(subs)} sub-issues of this epic are closed, so it is closed automatically "
        "(`Epic close` workflow, `tools/close_epics.py`). Reopen it if a new story is added.")})
    gh(f"repos/{repo}/issues/{number}", "-X", "PATCH",
       body={"state": "closed", "state_reason": "completed"})
    print(f"#{number} {epic['title']}: all {len(subs)} sub-issues closed, closed it.")
    return True


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    dry_run = os.environ.get("DRY_RUN", "").lower() == "true"
    issue = os.environ.get("ISSUE", "").strip()

    if issue:
        number = int(issue)
        for _ in range(MAX_LEVELS):
            parent = parent_of(repo, number)
            if parent is None:
                print(f"#{number} has no parent issue.")
                return 0
            if not try_close(repo, parent, dry_run):
                return 0
            number = parent["number"]
        return 0

    epics = [i for i in gh_pages(f"repos/{repo}/issues?state=open&labels={EPIC_LABEL}")
             if "pull_request" not in i]
    closed = sum(try_close(repo, epic, dry_run) for epic in epics)
    print(f"Checked {len(epics)} open epics; {'would close' if dry_run else 'closed'} {closed}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
