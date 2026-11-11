"""Fill a new pull request from the issue its branch was started for.

Runs from .github/workflows/pr-fill.yml when a pull request is opened. The branch name carries
the issue number (see tools/start_task.py). From that issue it adds, only where the PR does not
already have them: a `Closes #<issue>` line, the issue's labels, its milestone, its assignees,
and a Conventional Commit title `<type>(<story id or #issue>): <issue title>` when the current
title is not one. Rules: docs/guidelines/git-workflow.md
"""
import json
import os
import re
import subprocess
import sys

BRANCH = re.compile(r"^(?:([a-z]+)/)?(\d+)-[a-z0-9]")
TYPES = ("feat", "fix", "test", "refactor", "perf", "docs", "build", "ci", "chore", "research",
         "revert")
CONVENTIONAL = re.compile(rf"^({'|'.join(TYPES)})(\([^)]*\))?!?: \S")
CLOSES = re.compile(r"\b(close[sd]?|fix(e[sd])?|resolve[sd]?) #\d+", re.IGNORECASE)
STORY_ID = re.compile(r"^\s*(US-\d+(?:\.\d+)*|EP-\d+)(\s*\(in part\))?\s*", re.IGNORECASE)


def gh(*args, body=None):
    cmd = ["gh", "api", *args]
    if body is not None:
        cmd += ["--input", "-"]
    result = subprocess.run(cmd, input=json.dumps(body) if body is not None else None,
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"gh api {' '.join(args[:2])} failed: {result.stderr.strip()}")
    return json.loads(result.stdout) if result.stdout.strip() else None


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


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    with open(os.environ["GITHUB_EVENT_PATH"], encoding="utf-8") as f:
        pr = json.load(f)["pull_request"]
    match = BRANCH.match(pr["head"]["ref"])
    if not match:
        print(f"Skipping: branch '{pr['head']['ref']}' carries no issue number.")
        return
    branch_type, number = match.group(1), int(match.group(2))
    try:
        issue = gh(f"repos/{repo}/issues/{number}")
    except RuntimeError:
        print(f"Skipping: #{number} does not exist.")
        return
    if issue.get("pull_request"):
        print(f"Skipping: #{number} is a pull request, not an issue.")
        return

    issue_labels = [label["name"] for label in issue["labels"]]
    pr_labels = {label["name"] for label in pr.get("labels", [])}
    update = {}
    body = pr.get("body") or ""
    if not CLOSES.search(body):
        update["body"] = f"Closes #{number}\n\n{body}".rstrip() + "\n"
    if not CONVENTIONAL.match(pr["title"]):
        update["title"] = title_for(type_from(branch_type, set(issue_labels)), number,
                                    issue["title"])
    if update:
        gh("-X", "PATCH", f"repos/{repo}/pulls/{pr['number']}", body=update)

    extra = [label for label in issue_labels if label not in pr_labels and label != "type:epic"]
    if extra:
        gh("-X", "POST", f"repos/{repo}/issues/{pr['number']}/labels", body={"labels": extra})
    if issue.get("milestone") and not pr.get("milestone"):
        gh("-X", "PATCH", f"repos/{repo}/issues/{pr['number']}",
           body={"milestone": issue["milestone"]["number"]})
    if not pr.get("assignees"):
        people = [a["login"] for a in issue["assignees"]] or [pr["user"]["login"]]
        gh("-X", "POST", f"repos/{repo}/issues/{pr['number']}/assignees", body={"assignees": people})
    print(f"PR #{pr['number']} filled from #{number}: {sorted(update) + (['labels'] if extra else [])}")


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as error:
        print(f"::error::{error}")
        sys.exit(1)
