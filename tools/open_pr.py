"""Open the draft pull request for the current branch, filled from its issue.

Run it from the work branch after the first push (agents run it right after their first
approved commit is pushed; DEC-054):

    python tools/open_pr.py [--body-file BODY.md] [--title "feat(US-1.1): ..."]

The branch name carries the story id or issue number (tools/issue_ref.py). From that issue the
PR gets a `Closes #<issue>` line, the issue's labels (not `type:epic`), its milestone, its
assignees (or you), and a Conventional Commit title `<type>(<story id or #issue>): <issue title>`
unless --title is given. A docs, ci or chore branch may have no issue (DEC-056): its PR gets no
`Closes` line and the title `<type>: <branch description>` unless --title is given. The body is
--body-file, or the PR template. GitHub cannot open a PR
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

from issue_ref import branch_type, find_issue, may_skip_issue

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
    """Put `Closes #<number>` in the body (replacing the template's placeholder line), or drop
    the placeholder line when there is no issue."""
    if CLOSES.search(body):
        return body
    lines = body.splitlines()
    for i, line in enumerate(lines):
        if line.startswith("Closes #"):
            if number is None:
                del lines[i]
            else:
                lines[i] = f"Closes #{number}"
            return "\n".join(lines).lstrip("\n") + "\n"
    if number is None:
        return body
    return f"Closes #{number}\n\n{body}".rstrip() + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--body-file", type=pathlib.Path, help="PR body (default: the template)")
    parser.add_argument("--title", help="PR title (default: built from the issue)")
    args = parser.parse_args()

    repo = run("gh", "repo", "view", "--json", "nameWithOwner", "-q", ".nameWithOwner")
    branch = run("git", "rev-parse", "--abbrev-ref", "HEAD")
    existing = json.loads(run("gh", "pr", "list", "--head", branch, "--state", "open",
                              "--json", "number,url"))
    if existing:
        print(f"Branch '{branch}' already has PR #{existing[0]['number']}: {existing[0]['url']}")
        return

    kind = branch_type(branch)
    issue = find_issue(gh, repo, branch)
    if issue is None and not may_skip_issue(branch):
        raise RuntimeError(f"No issue found for branch '{branch}' (see docs/guidelines/git-workflow.md).")
    if issue is None:
        number, labels, milestone, assignees = None, [], None, ["@me"]
        description = branch.partition("/")[2].replace("-", " ")
        description = re.sub(r"^\d+ ", "", description)
        title = args.title or f"{kind}: {description[:1].upper()}{description[1:]}"
    else:
        number = issue["number"]
        labels = [label["name"] for label in issue["labels"] if label["name"] != "type:epic"]
        milestone = issue["milestone"]["title"] if issue.get("milestone") else None
        assignees = [a["login"] for a in issue["assignees"]] or ["@me"]
        title = args.title or title_for(type_from(kind, set(labels)), number, issue["title"])
    if not CONVENTIONAL.match(title):
        raise RuntimeError(f"Title is not a Conventional Commit: {title!r}")
    source = args.body_file if args.body_file else TEMPLATE
    body = body_for(number, source.read_text(encoding="utf-8"))

    cmd = ["gh", "pr", "create", "--draft", "--base", "main", "--head", branch,
           "--title", title, "--body-file", "-"]
    for label in labels:
        cmd += ["--label", label]
    if milestone:
        cmd += ["--milestone", milestone]
    for person in assignees:
        cmd += ["--assignee", person]
    result = subprocess.run(cmd, input=body, capture_output=True, text=True, encoding="utf-8")
    if result.returncode != 0:
        raise RuntimeError(f"gh pr create failed: {result.stderr.strip()}")
    print(f"Opened draft PR{f' for #{number}' if number else ''}: {result.stdout.strip()}")


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as error:
        print(f"error: {error}", file=sys.stderr)
        sys.exit(1)
