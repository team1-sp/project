"""Open the draft pull request for the current branch, filled from its issue.

Run it from the work branch after the first push (agents run it right after their first
approved commit is pushed; DEC-054):

    python tools/open_pr.py [--body-file BODY.md] [--title "feat(US-1.1): ..."]

The branch name carries the story id or issue number (tools/issue_ref.py). From that issue the
PR gets a `Closes #<issue>` line, the issue's labels (not `type:epic`), its milestone, its
assignees (or you), and a Conventional Commit title `<type>(<story id or #issue>): <issue title>`
unless --title is given.
A docs, ci or chore branch whose number has no issue is a branch named after its own PR number
(`ci/131-update-agent-workflow`): its PR gets the `no-issue` label, no `Closes` line, and the
title `ci(#131): Update agent workflow`. The number should be the PR's own; it warns if not. The body is --body-file, or the PR template. GitHub cannot open a PR
for a branch with no commits beyond main, so push a commit first. If the branch already has an
open PR, it prints that PR and does nothing. Needs the GitHub CLI (`gh`), signed in.
Rules: docs/guidelines/git-workflow.md
"""
import argparse
import json
import pathlib
import re
import subprocess
import sys

from issue_ref import find_issue, parse

NO_ISSUE_TYPES = ("docs", "ci", "chore")

TYPES = ("feat", "fix", "test", "refactor", "perf", "docs", "build", "ci", "chore", "research",
         "revert")
CONVENTIONAL = re.compile(rf"^({'|'.join(TYPES)})(\([^)]*\))?!?: \S")
CLOSES = re.compile(r"\b(close[sd]?|fix(e[sd])?|resolve[sd]?) #\d+", re.IGNORECASE)
STORY_ID = re.compile(r"^\s*(US-\d+(?:\.\d+)*|EP-\d+)(\s*\(in part\))?\s*", re.IGNORECASE)
TEMPLATE = pathlib.Path(__file__).resolve().parent.parent / ".github" / "pull_request_template.md"


def run(*cmd):
    result = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8")
    if result.returncode != 0:
        raise RuntimeError(f"{' '.join(cmd[:3])} failed: {result.stderr.strip()}")
    return result.stdout.strip()


def gh(*args):
    out = run("gh", "api", *args)
    return json.loads(out) if out else None


def type_from(branch_type, labels):
    if branch_type in TYPES:
        return branch_type
    if "type:research" in labels or "area:ai-research" in labels:
        return "research"
    if "type:bug" in labels:
        return "fix"
    if "area:devops" in labels and "type:story" not in labels:
        return "ci"
    return "feat"


def title_for(kind, issue_number, issue_title):
    match = STORY_ID.match(issue_title)
    scope = match.group(1).upper() if match else f"#{issue_number}"
    description = STORY_ID.sub("", issue_title).strip() if match else issue_title.strip()
    return f"{kind}({scope}): {description}"


def body_for(number, body):
    """Put `Closes #<number>` in the body (replacing the template's placeholder line). With no
    issue (number None) the placeholder line is dropped; the PR carries `no-issue` instead."""
    if CLOSES.search(body):
        return body
    lines = body.splitlines()
    for i, line in enumerate(lines):
        if line.startswith("Closes #"):
            if number is None:
                del lines[i]
            else:
                lines[i] = f"Closes #{number}"
            return "\n".join(lines) + "\n"
    if number is None:
        return body
    return f"Closes #{number}\n\n{body}".rstrip() + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--body-file", type=pathlib.Path, help="PR body (default: the template)")
    parser.add_argument("--title", help="PR title (default: built from the issue)")
    parser.add_argument("--dry-run", action="store_true", help="print the title, labels and body, create nothing")
    args = parser.parse_args()

    repo = run("gh", "repo", "view", "--json", "nameWithOwner", "-q", ".nameWithOwner")
    branch = run("git", "rev-parse", "--abbrev-ref", "HEAD")
    existing = json.loads(run("gh", "pr", "list", "--head", branch, "--state", "open",
                              "--json", "number,url"))
    if existing and not args.dry_run:
        print(f"Branch '{branch}' already has PR #{existing[0]['number']}: {existing[0]['url']}")
        return

    issue = find_issue(gh, repo, branch)
    kind, _, branch_number = parse(branch)
    own_number = None  # set for a docs/ci/chore branch named after its own PR number
    if issue is None:
        if kind not in NO_ISSUE_TYPES or branch_number is None:
            raise RuntimeError(f"No issue found for branch '{branch}' (see docs/guidelines/git-workflow.md).")
        own_number = branch_number
        description = re.sub(r"^[a-z]+/\d+-", "", branch).replace("-", " ")
        issue = {"number": None, "labels": [{"name": "no-issue"}], "assignees": [],
                 "title": f"{description[:1].upper()}{description[1:]}"}
    number = issue["number"]
    labels = [label["name"] for label in issue["labels"] if label["name"] != "type:epic"]

    if own_number is not None:
        title = args.title or f"{kind}(#{own_number}): {issue['title']}"
    else:
        title = args.title or title_for(type_from(kind, set(labels)), number, issue["title"])
    if not CONVENTIONAL.match(title):
        raise RuntimeError(f"Title is not a Conventional Commit: {title!r}")
    source = args.body_file if args.body_file else TEMPLATE
    body = body_for(number, source.read_text(encoding="utf-8"))

    cmd = ["gh", "pr", "create", "--draft", "--base", "main", "--head", branch,
           "--title", title, "--body-file", "-"]
    for label in labels:
        cmd += ["--label", label]
    if issue.get("milestone"):
        cmd += ["--milestone", issue["milestone"]["title"]]
    assignees = [a["login"] for a in issue["assignees"]] or ["@me"]
    for person in assignees:
        cmd += ["--assignee", person]
    if args.dry_run:
        print(f"title: {title}\nlabels: {labels}\ncommand: {' '.join(cmd)}\n--- body ---\n{body}")
        return
    result = subprocess.run(cmd, input=body, capture_output=True, text=True, encoding="utf-8")
    if result.returncode != 0:
        raise RuntimeError(f"gh pr create failed: {result.stderr.strip()}")
    url = result.stdout.strip()
    if own_number is not None:
        opened = int(url.rstrip("/").rsplit("/", 1)[1])
        if opened != own_number:
            print(f"warning: the branch says #{own_number} but the PR is #{opened}; rename the "
                  f"branch and the title scope to #{opened}.", file=sys.stderr)
        print(f"Opened draft PR #{opened} (no issue, labelled no-issue): {url}")
    else:
        print(f"Opened draft PR for #{number}: {url}")


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as error:
        print(f"error: {error}", file=sys.stderr)
        sys.exit(1)
