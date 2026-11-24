"""Remind the author of a pull request with other PRs stacked on it to merge it with a merge commit (DEC-158.1).

A PR is "stacked" on another when its base branch is the other PR's head branch. If the lower PR
is squash-merged, `main` gets its changes as one new commit while the stacked branch still holds
the original commits, so the stacked PR conflicts with `main` on files that are already identical
(seen with #154 -> #155 -> #156). A merge commit keeps the original commits on `main`, and GitHub
retargets the stacked PR to `main` without a conflict when the lower branch is deleted.

Runs from .github/workflows/stack-flag.yml when a PR is opened, reopened, retargeted or closed.
Every open PR with at least one open PR stacked on it gets the `has-stacked-prs` label and one
comment per stacked PR; the label is removed once nothing is stacked on it any more.
It never pushes, merges or changes a merge method. GitHub cannot pick the merge method for a PR,
so this is a reminder, not a check.

Local use: GITHUB_REPOSITORY=team1-sp/project DRY_RUN=1 python3 tools/flag_stacks.py
Rules: docs/guidelines/git-workflow.md (Stacked pull requests)
"""
import json
import os
import subprocess
import sys

LABEL = "has-stacked-prs"
MARKER = "<!-- stack-merge-reminder:#{child} -->"
COMMENT = (
    "{marker}\n"
    "Pull request #{child} is stacked on this one: its base is this PR's branch `{branch}`.\n\n"
    "**Merge this PR with \"Create a merge commit\", not \"Squash and merge\"** (DEC-158.1), and give "
    "the merge commit this PR's title. A squash would put this branch on `main` as one new commit "
    "while #{child} still holds the original commits, so #{child} would then conflict with `main` "
    "on files that are already the same. With a merge commit, GitHub moves #{child} onto `main` "
    "cleanly when this branch is deleted. Pull requests with nothing stacked on them are still "
    "squash-merged.\n\n"
    "Rules: `docs/guidelines/git-workflow.md` (Stacked pull requests). The `{label}` label goes "
    "away by itself once nothing is stacked on this PR."
)


def gh(*args, body=None):
    result = subprocess.run(["gh", *args], input=body, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"gh {' '.join(args[:3])} failed: {result.stderr.strip()}")
    return result.stdout


def api_list(path):
    """GET every page of a REST list endpoint."""
    out = gh("api", "--paginate", "--slurp", path)
    pages = json.loads(out)
    return [item for page in pages for item in page]


def stacks(prs, repo):
    """Map each open PR number to the open PRs whose base is its head branch (same repository only)."""
    by_head = {
        pr["head"]["ref"]: pr
        for pr in prs
        if pr["head"]["repo"] and pr["head"]["repo"]["full_name"] == repo
    }
    children = {}
    for pr in prs:
        parent = by_head.get(pr["base"]["ref"])
        if parent and parent["number"] != pr["number"]:
            children.setdefault(parent["number"], []).append(pr["number"])
    return children


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    dry = os.environ.get("DRY_RUN") == "1"
    prs = api_list(f"repos/{repo}/pulls?state=open&per_page=100")
    children = stacks(prs, repo)
    for pr in prs:
        number = pr["number"]
        labelled = any(label["name"] == LABEL for label in pr["labels"])
        stacked = sorted(children.get(number, []))
        if stacked:
            if not labelled:
                print(f"PR #{number}: has stacked PRs {stacked}, adding {LABEL}")
                if not dry:
                    gh("api", "-X", "POST", f"repos/{repo}/issues/{number}/labels",
                       "-f", f"labels[]={LABEL}")
            existing = "\n".join(c["body"] or "" for c in api_list(
                f"repos/{repo}/issues/{number}/comments?per_page=100"))
            for child in stacked:
                marker = MARKER.format(child=child)
                if marker in existing:
                    continue
                print(f"PR #{number}: reminding about stacked PR #{child}")
                if not dry:
                    body = COMMENT.format(marker=marker, child=child,
                                          branch=pr["head"]["ref"], label=LABEL)
                    gh("api", "-X", "POST", f"repos/{repo}/issues/{number}/comments",
                       "--input", "-", body=json.dumps({"body": body}))
        elif labelled:
            print(f"PR #{number}: nothing stacked on it any more, removing {LABEL}")
            if not dry:
                gh("api", "-X", "DELETE", f"repos/{repo}/issues/{number}/labels/{LABEL}")
    if not children:
        print("No open PR has another open PR stacked on it.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
