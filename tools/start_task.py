"""Create the work branch for issues that moved to "In progress" on the GitHub project.

Runs from .github/workflows/issue-start.yml. It only creates branches; developers open
their own pull requests. Rules: docs/guidelines/git-workflow.md

Two ways in:
- Scheduled or manual run without an issue number: reads the project (PROJECT_OWNER,
  PROJECT_NUMBER) with PROJECT_TOKEN and starts every open issue of this repository whose
  Status is STATUS_NAME ("In progress") and that has no branch yet.
- Manual run with ISSUE_NUMBER: starts that one issue, whatever its project status.

Branch name: <type>/<issue number>-<short description>, e.g. feat/41-run-every-simulation-tick.
With PROJECT_TOKEN the branch is created as a linked branch, so it shows under the issue's
Development section. Without it the branch is created with GITHUB_TOKEN and is not linked.
"""
import json
import os
import re
import subprocess
import sys
import unicodedata

SRS_ID = re.compile(r"^\s*(US-\d+(\.\d+)*|EP-\d+)(\s*\(in part\))?\s*", re.IGNORECASE)
TURKISH = str.maketrans("ıİşŞğĞüÜöÖçÇ", "iIsSgGuUoOcC")
MAX_SLUG = 40
SKIP_LABELS = {"type:epic"}  # epics are tracked through their stories, never worked on directly


def gh(*args, body=None, token=None):
    cmd = ["gh", "api", *args]
    if body is not None:
        cmd += ["--input", "-"]
    env = dict(os.environ)
    if token:
        env["GH_TOKEN"] = token
    result = subprocess.run(cmd, input=json.dumps(body) if body is not None else None,
                            capture_output=True, text=True, env=env)
    if result.returncode != 0:
        raise RuntimeError(f"gh api {' '.join(args[:2])} failed: {result.stderr.strip()}")
    return json.loads(result.stdout) if result.stdout.strip() else None


def graphql(query, token, **variables):
    body = {"query": query, "variables": variables}
    data = gh("graphql", body=body, token=token)
    if data.get("errors"):
        raise RuntimeError(f"GraphQL error: {data['errors']}")
    return data["data"]


def slugify(text):
    text = SRS_ID.sub("", text)
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


def branch_type(labels):
    if "type:research" in labels or "area:ai-research" in labels:
        return "research"
    if "type:bug" in labels:
        return "fix"
    if "area:docs" in labels and not labels & {"type:feature", "type:story", "type:bug"}:
        return "docs"
    if "area:devops" in labels and "type:story" not in labels:
        return "ci"
    return "feat"


def branch_name(number, title, labels):
    return f"{branch_type(labels)}/{number}-{slugify(title)}"


def existing_branch(repo, number):
    """Return a branch already started for this issue (<type>/<number>-...), if any."""
    pattern = re.compile(rf"^[a-z]+/{number}-")
    for branch in gh(f"repos/{repo}/branches?per_page=100", "--paginate"):
        if pattern.match(branch["name"]):
            return branch["name"]
    return None


PROJECT_ITEMS = """
query($owner: String!, $number: Int!, $status: String!, $cursor: String) {
  organization(login: $owner) {
    projectV2(number: $number) {
      items(first: 100, after: $cursor) {
        pageInfo { hasNextPage endCursor }
        nodes {
          fieldValueByName(name: $status) {
            ... on ProjectV2ItemFieldSingleSelectValue { name }
          }
          content {
            ... on Issue {
              id number title state
              repository { nameWithOwner }
              labels(first: 30) { nodes { name } }
              linkedBranches(first: 5) { nodes { ref { name } } }
            }
          }
        }
      }
    }
  }
}"""

CREATE_LINKED_BRANCH = """
mutation($issue: ID!, $repo: ID!, $oid: GitObjectID!, $name: String!) {
  createLinkedBranch(input: {issueId: $issue, repositoryId: $repo, oid: $oid, name: $name}) {
    linkedBranch { ref { name } }
  }
}"""


def in_progress_issues(repo, token):
    owner = os.environ.get("PROJECT_OWNER", repo.split("/")[0])
    number = int(os.environ.get("PROJECT_NUMBER", "3"))
    wanted = os.environ.get("STATUS_NAME", "In progress").casefold()
    cursor = None
    while True:
        data = graphql(PROJECT_ITEMS, token, owner=owner, number=number, status="Status",
                       cursor=cursor)
        items = data["organization"]["projectV2"]["items"]
        for node in items["nodes"]:
            issue = node.get("content") or {}
            status = (node.get("fieldValueByName") or {}).get("name", "")
            if (issue.get("number") and issue["repository"]["nameWithOwner"] == repo
                    and issue["state"] == "OPEN" and status.casefold() == wanted):
                yield issue
        if not items["pageInfo"]["hasNextPage"]:
            return
        cursor = items["pageInfo"]["endCursor"]


def start(repo, issue, labels, token, linked=()):
    number, title = issue["number"], issue["title"]
    if labels & SKIP_LABELS:
        print(f"Skipping #{number}: epics are worked on through their stories.")
        return
    if linked:
        print(f"Skipping #{number}: already has branch {linked[0]}.")
        return
    found = existing_branch(repo, number)
    if found:
        print(f"Skipping #{number}: already has branch {found}.")
        return

    branch = branch_name(number, title, labels)
    repo_info = gh(f"repos/{repo}")
    default = repo_info["default_branch"]
    sha = gh(f"repos/{repo}/git/ref/heads/{default}")["object"]["sha"]
    if token:
        graphql(CREATE_LINKED_BRANCH, token, issue=issue["id"], repo=repo_info["node_id"],
                oid=sha, name=branch)
    else:
        gh("-X", "POST", f"repos/{repo}/git/refs",
           body={"ref": f"refs/heads/{branch}", "sha": sha})

    gh("-X", "POST", f"repos/{repo}/issues/{number}/comments", body={"body": (
        f"Branch `{branch}` is ready.\n\n"
        f"```\ngit fetch origin\ngit switch {branch}\n```\n\n"
        f"When you open the pull request, put `Closes #{number}` in its description."
    )})
    print(f"Created {branch} for #{number}.")


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    token = os.environ.get("PROJECT_TOKEN", "").strip() or None
    one = os.environ.get("ISSUE_NUMBER", "").strip()

    if one:
        data = gh(f"repos/{repo}/issues/{one}")
        if data.get("pull_request"):
            raise RuntimeError(f"#{one} is a pull request, not an issue.")
        issue = {"number": data["number"], "title": data["title"], "id": data["node_id"]}
        start(repo, issue, {label["name"] for label in data["labels"]}, token)
        return

    if not token:
        print("::notice::PROJECT_TOKEN is not set, so the project cannot be read. "
              "Add the secret, or run the workflow by hand with an issue number.")
        return
    for issue in in_progress_issues(repo, token):
        labels = {label["name"] for label in issue["labels"]["nodes"]}
        linked = [b["ref"]["name"] for b in issue["linkedBranches"]["nodes"] if b.get("ref")]
        start(repo, issue, labels, token, linked)


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as error:
        print(f"::error::{error}")
        sys.exit(1)
