"""React to a new work branch: mark its issue as started.

Runs from .github/workflows/issue-start.yml on every branch creation. The branch name names
the issue: <type>/us-<x.y>-<short-description> for a user story (feat/us-15.2-build-settings),
or <type>/<issue>-<short-description> for any other issue (tools/issue_ref.py). Other branches
are skipped. Rules: docs/guidelines/git-workflow.md

For that issue it:
- moves the issue's card on the GitHub project to STATUS_NAME ("In progress"), unless the card
  is already further along (needs PROJECT_TOKEN; skipped without it),
- assigns the person who created the branch when nobody is assigned,
- comments on the issue with the branch and the checkout command.
It never creates branches or pull requests.
"""
import json
import os
import subprocess
import sys

from issue_ref import find_issue

EARLY_STATUSES = {"", "backlog", "ready", "todo", "to do"}  # statuses a new branch may move on from


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
    data = gh("graphql", body={"query": query, "variables": variables}, token=token)
    if data.get("errors"):
        raise RuntimeError(f"GraphQL error: {data['errors']}")
    return data["data"]


PROJECT = """
query($owner: String!, $number: Int!) {
  organization(login: $owner) {
    projectV2(number: $number) {
      id
      field(name: "Status") {
        ... on ProjectV2SingleSelectField { id options { id name } }
      }
    }
  }
}"""

ISSUE_ITEMS = """
query($node: ID!) {
  node(id: $node) {
    ... on Issue {
      projectItems(first: 20) {
        nodes {
          id
          project { id }
          fieldValueByName(name: "Status") {
            ... on ProjectV2ItemFieldSingleSelectValue { name }
          }
        }
      }
    }
  }
}"""

ADD_ITEM = """
mutation($project: ID!, $content: ID!) {
  addProjectV2ItemById(input: {projectId: $project, contentId: $content}) { item { id } }
}"""

SET_STATUS = """
mutation($project: ID!, $item: ID!, $field: ID!, $option: String!) {
  updateProjectV2ItemFieldValue(input: {projectId: $project, itemId: $item, fieldId: $field,
                                        value: {singleSelectOptionId: $option}}) {
    projectV2Item { id }
  }
}"""


def move_to_in_progress(issue_node_id, token):
    """Set the issue's project Status; return a short note on what happened."""
    owner = os.environ.get("PROJECT_OWNER", "team1-sp")
    number = int(os.environ.get("PROJECT_NUMBER", "3"))
    wanted = os.environ.get("STATUS_NAME", "In progress")

    project = graphql(PROJECT, token, owner=owner, number=number)["organization"]["projectV2"]
    field = project["field"]
    option = next((o["id"] for o in field["options"] if o["name"].casefold() == wanted.casefold()),
                  None)
    if not option:
        raise RuntimeError(f"The project's Status field has no option named '{wanted}'.")

    items = graphql(ISSUE_ITEMS, token, node=issue_node_id)["node"]["projectItems"]["nodes"]
    item = next((i for i in items if i["project"]["id"] == project["id"]), None)
    if item is None:
        item_id = graphql(ADD_ITEM, token, project=project["id"],
                          content=issue_node_id)["addProjectV2ItemById"]["item"]["id"]
        current = ""
    else:
        item_id = item["id"]
        current = (item.get("fieldValueByName") or {}).get("name", "")
    if current.casefold() not in EARLY_STATUSES:
        return f"left at {current}"
    graphql(SET_STATUS, token, project=project["id"], item=item_id, field=field["id"],
            option=option)
    return f"moved to {wanted}"


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    branch = os.environ["BRANCH"]
    actor = os.environ.get("ACTOR", "")
    token = os.environ.get("PROJECT_TOKEN", "").strip() or None

    issue = find_issue(gh, repo, branch)
    if issue is None:
        print(f"Skipping '{branch}': no issue found for this branch name.")
        return
    number = issue["number"]
    if issue["state"] != "open":
        print(f"Skipping '{branch}': #{number} is closed.")
        return
    if any(label["name"] == "type:epic" for label in issue["labels"]):
        print(f"Skipping '{branch}': #{number} is an epic; work on one of its stories.")
        return

    if token:
        status = move_to_in_progress(issue["node_id"], token)
    else:
        status = "not changed (the PROJECT_TOKEN secret is not set)"
        print(f"::notice::Project status {status}.")

    if not issue["assignees"] and actor and not actor.endswith("[bot]"):
        gh("-X", "POST", f"repos/{repo}/issues/{number}/assignees", body={"assignees": [actor]})

    gh("-X", "POST", f"repos/{repo}/issues/{number}/comments", body={"body": (
        f"Work started on branch `{branch}`"
        + (f" by @{actor}" if actor else "") + f". Project status: {status}.\n\n"
        f"```\ngit fetch origin\ngit switch {branch}\n```\n\n"
        "When you open the pull request, its title, `Closes` line, labels and milestone are "
        "filled in from this issue."
    )})
    print(f"#{number}: status {status}.")


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as error:
        print(f"::error::{error}")
        sys.exit(1)
