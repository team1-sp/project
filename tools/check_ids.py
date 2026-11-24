"""Check the ids of decision, problem, handoff, question and GenAI records (DEC-055, DEC-148.1, DEC-152.1).

New entries use `<PREFIX>-<GitHub #>.<counter>`, for example DEC-148.1 or HND-80.2, where the
number is the issue the pull request closes, or the pull request's own number for no-issue work.
Tasks are GitHub issues and get no id (DEC-149.1). Entries written before DEC-148.1 keep their
3-digit ids (DEC-071, HND-042) and are never renumbered.

Always checked (CI and the .githooks hooks):
- every heading id has the old or the new form;
- no id is used by two headings, across a record's live and archive files (two appends can merge
  without a git conflict and still collide, and a `merge=union` merge can keep a line twice);
- every decision and problem entry has exactly one index row, and every index row an entry;
- decision index rows are one line of at most 120 characters, and only superseded or rejected
  decisions sit in its Archived table (DEC-152.1);
- the live handoff log holds at most 15 entries (rotate with tools/rotate_handoffs.py, DEC-152.1).

With --base <rev> (pull request CI): every id this branch adds compared with <rev> must use the
pull request's number, read from the event in GITHUB_EVENT_PATH: a `Closes #n` issue, or the pull
request number when it has the `no-issue` label.

Usage: python tools/check_ids.py [repo root] [--base <rev>]
"""
import argparse
import collections
import fnmatch
import json
import os
import pathlib
import re
import subprocess
import sys

# prefix -> files holding its `### <ID> — ...` entries (globs allowed). An id must be unique across
# all files of its prefix: moving an entry to an archive file is fine, keeping it twice is not.
RECORDS = {
    "DEC": ["docs/history/decision-log.md"],
    "PRB": ["docs/history/problems/*.md"],
    "GEN": ["docs/history/genai-usage-log.md"],
    "HND": ["docs/agents/state/handoff-log.md", "docs/history/handoff-archive/*.md"],
    "Q": ["docs/agents/state/open-questions.md", "docs/history/questions-archive.md"],
}
# prefix -> the file holding its one-line index rows (DEC-152.1)
INDEXES = {
    "DEC": "docs/history/decision-index.md",
    "PRB": "docs/history/problems/README.md",
}
# Where record files used to live, so a moved file's entries do not count as new.
FORMER = {
    "PRB": ["docs/history/problem-log.md"],  # split into history/problems/ by DEC-152.1
}
LIVE_HANDOFFS = "docs/agents/state/handoff-log.md"
HANDOFF_LIMIT = 15  # tools/rotate_handoffs.py keeps 10; rotate when the live log passes this
DECISION_WIDTH = 120  # characters in the Decision column of the decision index
CLOSES = re.compile(r"\b(?:close[sd]?|fix(?:e[sd])?|resolve[sd]?) #(\d+)", re.IGNORECASE)
NUMBER = r"[1-9]\d*"


def id_forms(prefix):
    """Return (old form, new form) regexes for a prefix; group 1 of the new form is the #."""
    old = re.compile(rf"{prefix}-\d{{3}}")
    new = re.compile(rf"{prefix}-({NUMBER})\.{NUMBER}")
    return old, new


def headings(text, prefix):
    return re.findall(rf"^### ({prefix}-\d+(?:\.\d+)*)", text, re.MULTILINE)


def index_rows(text, prefix):
    return re.findall(rf"^\| ({prefix}-\d+(?:\.\d+)*) \|", text, re.MULTILINE)


def check_ids(prefix, texts):
    """Check the ids of one prefix across its files; `texts` maps file -> text."""
    errors = []
    old, new = id_forms(prefix)
    example = f"{prefix}-<GitHub #>.<counter>"
    seen = collections.Counter()
    for rel, text in texts.items():
        for record in headings(text, prefix):
            seen[record] += 1
            if not (old.fullmatch(record) or new.fullmatch(record)):
                errors.append(f"{rel}: {record} is not a valid id; new entries use {example}")
    for dup, count in seen.items():
        if count > 1:
            where = ", ".join(rel for rel, text in texts.items() if dup in headings(text, prefix))
            errors.append(f"{dup} is used by {count} entries ({where}); drop the doubled copy or renumber the later one")
    return errors


def check_index(prefix, rel, text, ids):
    """Every entry has exactly one index row and every row an entry; decision rows stay short."""
    errors = []
    rows = index_rows(text, prefix)
    for dup, count in collections.Counter(rows).items():
        if count > 1:
            errors.append(f"{rel}: {dup} has {count} index rows")
    for missing in sorted(set(ids) - set(rows)):
        errors.append(f"{rel}: {missing} has no index row")
    for orphan in sorted(set(rows) - set(ids)):
        errors.append(f"{rel}: index row {orphan} has no entry")
    if prefix == "DEC":
        errors += check_decision_rows(rel, text)
    return errors


def check_decision_rows(rel, text):
    """One-line rows of at most DECISION_WIDTH characters; archived rows are the closed ones."""
    errors = []
    section = None
    for line in text.splitlines():
        if line.startswith("## "):
            section = line[3:].strip()
            continue
        match = re.match(r"^\| (DEC-\d+(?:\.\d+)*) \| (.*?) \| (.*?) \| (.*?) \|$", line)
        if not match:
            continue
        record, decision, status = match.group(1), match.group(2), match.group(3)
        if len(decision) > DECISION_WIDTH:
            errors.append(f"{rel}: {record} row is {len(decision)} characters; keep the decision to one line of "
                          f"at most {DECISION_WIDTH} (the details belong in the entry, DEC-152.1)")
        closed = status.startswith(("Superseded", "Rejected"))
        if section == "Active" and closed:
            errors.append(f"{rel}: {record} is {status}; move its row to the Archived table")
        if section == "Archived" and not closed:
            errors.append(f"{rel}: {record} is {status}; only superseded or rejected rows belong in Archived")
    return errors


def check_live_handoffs(text):
    count = len(headings(text, "HND"))
    if count > HANDOFF_LIMIT:
        return [f"{LIVE_HANDOFFS} holds {count} entries (limit {HANDOFF_LIMIT}); run "
                "`python tools/rotate_handoffs.py` to move the older ones to docs/history/handoff-archive/"]
    return []


def expand(root, patterns, index=None):
    """Existing files matching the patterns, as repo-relative paths, without the index file."""
    files = []
    for pattern in patterns:
        for path in sorted(root.glob(pattern)):
            rel = path.relative_to(root).as_posix()
            if rel != index and path.is_file():
                files.append(rel)
    return files


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


def files_at(root, rev, patterns):
    result = subprocess.run(["git", "-C", str(root), "ls-tree", "-r", "--name-only", rev],
                            capture_output=True, text=True, encoding="utf-8")
    names = result.stdout.splitlines() if result.returncode == 0 else []
    return [n for n in names if any(fnmatch.fnmatch(n, p) for p in patterns)]


def base_ids(root, rev):
    """Return {prefix: set of heading ids} at the base revision, including former locations."""
    ids = collections.defaultdict(set)
    for prefix in RECORDS:
        patterns = RECORDS[prefix] + FORMER.get(prefix, [])
        for rel in files_at(root, rev, patterns):
            ids[prefix].update(headings(file_at(root, rev, rel), prefix))
    return ids


def check_added(rel, prefix, text, before, numbers):
    """Ids added since the base must use one of `numbers` in the new form."""
    errors = []
    _, new = id_forms(prefix)
    example = f"{prefix}-<n>.<counter>"
    allowed = " or ".join(f"#{n}" for n in sorted(numbers))
    for record in dict.fromkeys(headings(text, prefix)):
        if record in before:
            continue
        match = new.fullmatch(record)
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
    before = base_ids(root, args.base) if numbers else {}
    for prefix, patterns in RECORDS.items():
        index = INDEXES.get(prefix)
        texts = {rel: (root / rel).read_text(encoding="utf-8") for rel in expand(root, patterns, index)}
        errors += check_ids(prefix, texts)
        if index and (root / index).exists():
            ids = [i for text in texts.values() for i in headings(text, prefix)]
            errors += check_index(prefix, index, (root / index).read_text(encoding="utf-8"), ids)
        if numbers:
            for rel, text in texts.items():
                errors += check_added(rel, prefix, text, before[prefix], numbers)
    live = root / LIVE_HANDOFFS
    if live.exists():
        errors += check_live_handoffs(live.read_text(encoding="utf-8"))
    if args.base and not numbers:
        print("No 'Closes #n' or no-issue label found; new ids were not matched to the PR.")
    for error in errors:
        print(f"::error::{error}")
    if not errors:
        print("Record ids are valid and unique, and the indexes match.")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
