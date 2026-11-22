"""Check the ids of decision, problem, handoff, question, task and GenAI records (DEC-055, DEC-148.1).

New entries use `<PREFIX>-<GitHub #>.<counter>`, for example DEC-148.1 or HND-80.2, where the
number is the issue the pull request closes, or the pull request's own number for no-issue work.
Tasks are GitHub issues (DEC-149.1); the archived task board keeps its TASK-0nn ids. Entries written before DEC-148.1 keep their
3-digit ids (DEC-071, HND-042, TASK-025) and are never renumbered.

Always checked (CI and the .githooks hooks):
- every heading id has the old or the new form;
- no id is used by two headings (two appends can merge without a git conflict and still collide,
  and a `merge=union` merge can keep a line twice);
- every decision and problem entry has exactly one index row, and every index row an entry.

With --base <rev> (pull request CI): every id this branch adds compared with <rev> must use the
pull request's number, read from the event in GITHUB_EVENT_PATH: a `Closes #n` issue, or the pull
request number when it has the `no-issue` label.

Usage: python tools/check_ids.py [repo root] [--base <rev>]
"""
import argparse
import collections
import json
import os
import pathlib
import re
import subprocess
import sys

# file -> id prefix of its `### <ID> — ...` headings
RECORDS = {
    "docs/history/decision-log.md": "DEC",
    "docs/history/problem-log.md": "PRB",
    "docs/history/genai-usage-log.md": "GEN",
    "docs/agents/state/handoff-log.md": "HND",
    "docs/agents/state/open-questions.md": "Q",
    "docs/history/task-board-archive.md": "TASK",  # retired by DEC-149.1; kept for history
}
INDEXED = ("DEC", "PRB")
CLOSES = re.compile(r"\b(?:close[sd]?|fix(?:e[sd])?|resolve[sd]?) #(\d+)", re.IGNORECASE)
NUMBER = r"[1-9]\d*"


def id_forms(prefix):
    """Return (old form, new form) regexes for a prefix; group 1 of the new form is the #."""
    old = re.compile(rf"{prefix}-\d{{3}}")
    if prefix == "TASK":
        new = re.compile(rf"TASK-({NUMBER})")
    else:
        new = re.compile(rf"{prefix}-({NUMBER})\.{NUMBER}")
    return old, new


def headings(text, prefix):
    return re.findall(rf"^### ({prefix}-\d+(?:\.\d+)*)", text, re.MULTILINE)


def index_rows(text, prefix):
    return re.findall(rf"^\| ({prefix}-\d+(?:\.\d+)*) \|", text, re.MULTILINE)


def check_file(rel, prefix, text):
    errors = []
    old, new = id_forms(prefix)
    example = "no new TASK ids (DEC-149.1)" if prefix == "TASK" else f"{prefix}-<GitHub #>.<counter>"
    ids = headings(text, prefix)
    for record in ids:
        if not (old.fullmatch(record) or new.fullmatch(record)):
            errors.append(f"{rel}: {record} is not a valid id; new entries use {example}")
    for dup, count in collections.Counter(ids).items():
        if count > 1:
            errors.append(f"{rel}: {dup} is used by {count} entries; drop the doubled copy or renumber the later one")
    if prefix in INDEXED:
        rows = index_rows(text, prefix)
        for dup, count in collections.Counter(rows).items():
            if count > 1:
                errors.append(f"{rel}: {dup} has {count} index rows")
        if rows:
            for missing in sorted(set(ids) - set(rows)):
                errors.append(f"{rel}: {missing} has no index row")
            for orphan in sorted(set(rows) - set(ids)):
                errors.append(f"{rel}: index row {orphan} has no entry")
    return errors


def pr_numbers(event_path):
    """Return the GitHub numbers new ids may use, from a pull_request event (empty if none)."""
    if not event_path or not pathlib.Path(event_path).exists():
        return set()
    with open(event_path, encoding="utf-8") as f:
        pr = json.load(f).get("pull_request")
    if not pr:
        return set()
    numbers = {int(n) for n in CLOSES.findall(pr.get("body") or "")}
    if "no-issue" in {label["name"] for label in pr.get("labels", [])}:
        numbers.add(int(pr["number"]))
    return numbers


def file_at(root, rev, rel):
    result = subprocess.run(["git", "-C", str(root), "show", f"{rev}:{rel}"],
                            capture_output=True, text=True, encoding="utf-8")
    return result.stdout if result.returncode == 0 else ""


def check_added(rel, prefix, text, base_text, numbers):
    """Ids added since the base must use one of `numbers` in the new form."""
    errors = []
    _, new = id_forms(prefix)
    example = "TASK-<n>" if prefix == "TASK" else f"{prefix}-<n>.<counter>"
    allowed = " or ".join(f"#{n}" for n in sorted(numbers))
    before = set(headings(base_text, prefix))
    for record in dict.fromkeys(headings(text, prefix)):
        if record in before:
            continue
        match = new.fullmatch(record)  # TASK-025 never matches: a new # has no leading zero
        if match and int(match.group(1)) in numbers:
            continue
        errors.append(f"{rel}: new entry {record} must use this PR's number ({example} with n = "
                      f"{allowed}); DEC-148.1, git-workflow.md §4")
    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("root", nargs="?", default=".")
    parser.add_argument("--base", help="git revision the pull request merges into")
    args = parser.parse_args(argv)
    root = pathlib.Path(args.root)

    numbers = pr_numbers(os.environ.get("GITHUB_EVENT_PATH")) if args.base else set()
    errors = []
    for rel, prefix in RECORDS.items():
        path = root / rel
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        errors += check_file(rel, prefix, text)
        if numbers:
            errors += check_added(rel, prefix, text, file_at(root, args.base, rel), numbers)
    if args.base and not numbers:
        print("No 'Closes #n' or no-issue label found; new ids were not matched to the PR.")
    for error in errors:
        print(f"::error::{error}")
    if not errors:
        print("Record ids are valid and unique, and the indexes match.")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
