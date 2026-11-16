"""Close an epic issue once all of its stories are done; mark it when only later stories are left.

Runs from .github/workflows/epic-close.yml. Epics are the issues labelled `type:epic`
(EP-xx); their planned stories are GitHub sub-issues (DEC-050), and stories not planned yet are
unchecked items in the "Later stories" checklist of the epic's body. Rules (DEC-062):
docs/guidelines/git-workflow.md

For an open epic with at least one sub-issue:
- some sub-issue open      -> leave it open and take off the `increment-done` label;
- all sub-issues closed, but unchecked later stories remain
                           -> leave it open and add the `increment-done` label;
- all sub-issues closed and no later stories left
                           -> comment and close it as completed.
An epic with no sub-issues is left alone. Closed epics are never reopened.

When to check:
- ISSUE set (the workflow runs on `issues: closed, reopened, edited`): the issue itself if it
  is an epic (an edited "Later stories" list, a reopened epic), then its parent, and one level
  further up after a close, because a close made with the workflow's own token starts no other
  workflow run.
- ISSUE empty (run by hand, workflow_dispatch): every open epic.

Env: GH_TOKEN (issues: write), GITHUB_REPOSITORY, ISSUE (optional), DRY_RUN ("true" only prints).
"""
import json
import os
import re
import subprocess
import sys

EPIC_LABEL = "type:epic"
DONE_LABEL = "increment-done"
MAX_LEVELS = 5  # parent chain to walk up from one closed issue
LATER_HEADING = re.compile(r"^#{2,6}\s+Later stories\s*$", re.IGNORECASE | re.MULTILINE)
NEXT_HEADING = re.compile(r"^(#{1,6}\s|---\s*$)", re.MULTILINE)
UNCHECKED = re.compile(r"^\s*[-*+]\s+\[ \]\s+(.*)$", re.MULTILINE)
ISSUE_REF = re.compile(r"^(#\d+|https://github\.com/\S+/issues/\d+)\b")


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


def has_label(issue, name):
    return any(label["name"] == name for label in issue.get("labels", []))


def later_stories(body):
    """Unchecked items of the "Later stories" section that are not issue references.

    A story converted to a sub-issue is tracked as a sub-issue, so an item that only links an
    issue is not counted again.
    """
    match = LATER_HEADING.search(body or "")
    if not match:
        return []
    rest = body[match.end():]
    end = NEXT_HEADING.search(rest)
    section = rest[:end.start()] if end else rest
    return [item for item in UNCHECKED.findall(section) if not ISSUE_REF.match(item.strip())]


def set_done_label(repo, epic, wanted, dry_run):
    number, present = epic["number"], has_label(epic, DONE_LABEL)
    if wanted == present:
        return
    verb = "add" if wanted else "remove"
    if dry_run:
        print(f"#{number}: would {verb} the {DONE_LABEL} label (dry run).")
    elif wanted:
        gh(f"repos/{repo}/issues/{number}/labels", body={"labels": [DONE_LABEL]})
    else:
        gh(f"repos/{repo}/issues/{number}/labels/{DONE_LABEL}", "-X", "DELETE")


def check_epic(repo, epic, dry_run):
    """Apply the rules to one issue. True if it closed (or would close, in a dry run)."""
    number, title = epic["number"], epic["title"]
    if epic["state"] != "open" or not has_label(epic, EPIC_LABEL):
        print(f"#{number} {title}: not an open epic, nothing to do.")
        return False
    subs = gh_pages(f"repos/{repo}/issues/{number}/sub_issues")
    if not subs:
        print(f"#{number} {title}: no sub-issues, left open.")
        return False
    still_open = [s for s in subs if s["state"] == "open"]
    if still_open:
        listed = ", ".join(f"#{s['number']}" for s in still_open)
        print(f"#{number} {title}: {len(still_open)} of {len(subs)} sub-issues open ({listed}).")
        set_done_label(repo, epic, False, dry_run)
        return False
    later = later_stories(epic.get("body"))
    if later:
        print(f"#{number} {title}: all {len(subs)} sub-issues closed, {len(later)} later "
              f"stories left; marked {DONE_LABEL}.")
        set_done_label(repo, epic, True, dry_run)
        return False
    if dry_run:
        print(f"#{number} {title}: all {len(subs)} sub-issues closed, no later stories, would close (dry run).")
        return True
    set_done_label(repo, epic, False, dry_run)
    gh(f"repos/{repo}/issues/{number}/comments", body={"body": (
        f"All {len(subs)} sub-issues of this epic are closed and no later stories are left, so it "
        "is closed automatically (`Epic close` workflow, `tools/close_epics.py`, DEC-062).")})
    gh(f"repos/{repo}/issues/{number}", "-X", "PATCH",
       body={"state": "closed", "state_reason": "completed"})
    print(f"#{number} {title}: all {len(subs)} sub-issues closed, no later stories, closed it.")
    return True


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    dry_run = os.environ.get("DRY_RUN", "").lower() == "true"
    issue = os.environ.get("ISSUE", "").strip()

    if issue:
        current = gh(f"repos/{repo}/issues/{int(issue)}")
        if has_label(current, EPIC_LABEL):
            check_epic(repo, current, dry_run)
        number = current["number"]
        for _ in range(MAX_LEVELS):
            parent = parent_of(repo, number)
            if parent is None:
                print(f"#{number} has no parent issue.")
                return 0
            if not check_epic(repo, parent, dry_run):
                return 0
            number = parent["number"]
        return 0

    epics = [i for i in gh_pages(f"repos/{repo}/issues?state=open&labels={EPIC_LABEL}")
             if "pull_request" not in i]
    closed = sum(check_epic(repo, epic, dry_run) for epic in epics)
    print(f"Checked {len(epics)} open epics; {'would close' if dry_run else 'closed'} {closed}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
