"""Create the branch and draft PR for a backlog issue opened from ClickUp.

Runs from .github/workflows/issue-start.yml when an issue is opened or assigned.
Handles issues whose title carries a backlog id, e.g. "PB-025 Define the physics adapter
interface" or "ci: pb-001-set-up-repository"; anything else is skipped.
Rules: docs/guidelines/git-workflow.md
"""
import json
import os
import re
import subprocess
import sys
import unicodedata

PB_ID = re.compile(r"\bpb-?(\d{3})\b", re.IGNORECASE)
TYPE_PREFIX = re.compile(r"^(feat|fix|test|refactor|docs|ci|chore|research)(\([^)]*\))?:\s*", re.IGNORECASE)
CLICKUP_URL = re.compile(r"https://app\.clickup\.com/t/[A-Za-z0-9/_-]+")
TURKISH = str.maketrans("ıİşŞğĞüÜöÖçÇ", "iIsSgGuUoOcC")
MAX_SLUG = 40


def gh(*args, body=None):
    cmd = ["gh", "api", *args]
    if body is not None:
        cmd += ["--input", "-"]
    result = subprocess.run(cmd, input=json.dumps(body) if body is not None else None,
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"gh api {' '.join(args)} failed: {result.stderr.strip()}")
    return json.loads(result.stdout) if result.stdout.strip() else None


def slugify(text):
    text = unicodedata.normalize("NFKD", text.translate(TURKISH))
    text = text.encode("ascii", "ignore").decode().lower()
    words = re.findall(r"[a-z0-9]+", text)
    slug = ""
    for word in words:
        candidate = f"{slug}-{word}" if slug else word
        if len(candidate) > MAX_SLUG:
            break
        slug = candidate
    return slug or "task"


def parse_title(raw):
    """Return (pb id, type from a Conventional prefix or None, readable title)."""
    pb = PB_ID.search(raw)
    if not pb:
        return None, None, None
    prefix = TYPE_PREFIX.match(raw)
    kind = prefix.group(1).lower() if prefix else None
    rest = raw[prefix.end():] if prefix else raw
    rest = PB_ID.sub(" ", rest).strip(" :-_")
    if " " not in rest:  # slug-style title such as "set-up-repository"
        rest = re.sub(r"[_-]+", " ", rest)
    rest = re.sub(r"\s+", " ", rest).strip()
    return pb.group(1), kind, rest or raw.strip()


def branch_type(labels):
    if "type:research" in labels or "area:ai-research" in labels:
        return "research"
    if "type:bug" in labels:
        return "fix"
    if labels and labels <= {"area:docs", "type:chore", "priority:urgent", "priority:high",
                             "priority:normal", "priority:low"} and "area:docs" in labels:
        return "docs"
    if "area:devops" in labels:
        return "ci"
    return "feat"


def pr_body(issue_number, clickup_url):
    return "\n".join([
        f"Closes #{issue_number}",
        "",
        f"ClickUp: {clickup_url or '<!-- task link -->'}",
        "",
        "## What and why",
        "",
        "## How it was verified",
        "<!-- Commands you ran and their results. -->",
        "",
        "## Docs updated",
        "",
        "## Definition of Done",
        "- [ ] Acceptance criteria in the issue are met",
        "- [ ] Builds from a clean checkout; tests added or updated and passing",
        "- [ ] Tests run headless (no window or GPU)",
        "- [ ] Architecture rules R-1 to R-9 respected (`docs/agents/rules.md`)",
        "- [ ] No Box2D or raylib types outside their adapter targets",
        "- [ ] No new compiler warnings, no secrets, no large binaries",
        "- [ ] Docs and decision log updated where the change made them inaccurate",
        "",
        "After merging: set the ClickUp task to **done**.",
        "",
        "_Branch and draft PR created automatically when the issue was opened._",
    ])


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    with open(os.environ["GITHUB_EVENT_PATH"], encoding="utf-8") as f:
        issue = json.load(f)["issue"]

    pb, kind, title = parse_title(issue["title"].strip())
    if not pb:
        print(f"Skipping #{issue['number']}: title has no PB-<nnn> id.")
        return
    labels = {label["name"] for label in issue.get("labels", [])}
    kind = kind or branch_type(labels)
    branch = f"{kind}/pb-{pb}-{slugify(title)}"

    # One PR per backlog item: skip if a PR for this PB id already exists.
    for pr in gh(f"repos/{repo}/pulls?state=all&per_page=100", "--paginate"):
        if re.search(rf"\bpb-?{pb}\b", pr["title"] + " " + pr["head"]["ref"], re.IGNORECASE):
            print(f"Skipping #{issue['number']}: PB-{pb} already has PR #{pr['number']}.")
            return

    default = gh(f"repos/{repo}")["default_branch"]
    base_sha = gh(f"repos/{repo}/git/ref/heads/{default}")["object"]["sha"]
    tree = gh(f"repos/{repo}/git/commits/{base_sha}")["tree"]["sha"]
    # GitHub cannot open a PR for a branch identical to its base, so start with one empty
    # commit. Squash merging leaves it out of main's history.
    start = gh("-X", "POST", f"repos/{repo}/git/commits", body={
        "message": f"chore: start PB-{pb}",
        "tree": tree,
        "parents": [base_sha],
    })
    gh("-X", "POST", f"repos/{repo}/git/refs",
       body={"ref": f"refs/heads/{branch}", "sha": start["sha"]})

    clickup = CLICKUP_URL.search(issue.get("body") or "")
    pr = gh("-X", "POST", f"repos/{repo}/pulls", body={
        "title": f"{kind}: {title} (PB-{pb})",
        "head": branch,
        "base": default,
        "body": pr_body(issue["number"], clickup.group(0) if clickup else None),
        "draft": True,
    })

    people = [a["login"] for a in issue.get("assignees", [])] or [issue["user"]["login"]]
    people = [p for p in people if not p.endswith("[bot]")]
    if people:
        gh("-X", "POST", f"repos/{repo}/issues/{pr['number']}/assignees", body={"assignees": people})
    reviewers = [r for r in os.environ.get("DEFAULT_REVIEWERS", "").split() if r not in people]
    if reviewers:
        gh("-X", "POST", f"repos/{repo}/pulls/{pr['number']}/requested_reviewers",
           body={"reviewers": reviewers})

    gh("-X", "POST", f"repos/{repo}/issues/{issue['number']}/comments", body={"body": (
        f"Draft PR #{pr['number']} is ready on branch `{branch}`.\n\n"
        f"```\ngit fetch origin\ngit switch {branch}\n```"
    )})
    print(f"Created {branch} and draft PR #{pr['number']} for #{issue['number']}.")


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as error:
        print(f"::error::{error}")
        sys.exit(1)
