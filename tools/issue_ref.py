"""Find the issue a work branch belongs to. Shared by start_task.py and fill_pr.py.

Branch names (docs/guidelines/git-workflow.md):
- <type>/us-<x.y>-<short-description> for an SRS user story, e.g. feat/us-15.2-build-settings
- <type>/<issue>-<short-description> for any other issue, e.g. fix/142-door-opens-without-key
- <type>/pb-<nnn>-<short-description>, the old ClickUp form (no issue lookup)
"""
import re

TYPES = ("feat", "fix", "test", "refactor", "docs", "ci", "chore", "research")
# Keep in sync with the "branch-names" ruleset of the repository.
BRANCH = re.compile(
    rf"^({'|'.join(TYPES)})/(us-\d+\.\d+|\d+|pb-\d{{3}}(\.\d+)?)-[a-z0-9]+(-[a-z0-9]+)*$"
)
REF = re.compile(rf"^({'|'.join(TYPES)})/(?:us-(\d+\.\d+)|(\d+))-")


def parse(branch):
    """Return (type, story id like 'US-15.2' or None, issue number or None)."""
    match = REF.match(branch)
    if not match:
        return None, None, None
    story = f"US-{match.group(2)}" if match.group(2) else None
    number = int(match.group(3)) if match.group(3) else None
    return match.group(1), story, number


def find_issue(gh, repo, branch):
    """Return the issue (REST JSON) for a branch, or None. `gh` is the caller's API helper."""
    _, story, number = parse(branch)
    if number is not None:
        try:
            issue = gh(f"repos/{repo}/issues/{number}")
        except RuntimeError:
            return None
        return None if issue.get("pull_request") else issue
    if story:
        prefix = re.compile(rf"^{re.escape(story)}(\s|$)", re.IGNORECASE)
        for issue in gh(f"repos/{repo}/issues?state=all&labels=type:story&per_page=100",
                        "--paginate"):
            if prefix.match(issue["title"]) and not issue.get("pull_request"):
                return issue
    return None
