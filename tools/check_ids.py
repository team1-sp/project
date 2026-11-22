"""Fail when two branches gave records the same id (DEC-055).

Two pull requests that each append an entry to the same log usually merge without a git
conflict, but both entries may carry the same number. This check reads the record files and
reports any id used for two headings, and any decision-log entry missing from its index (or the
other way round). Usage: python tools/check_ids.py [repo root]
"""
import collections
import pathlib
import re
import sys

# file -> id prefix of its `### <ID> — ...` headings
RECORDS = {
    "docs/history/decision-log.md": "DEC",
    "docs/history/problem-log.md": "PRB",
    "docs/agents/state/handoff-log.md": "HND",
    "docs/agents/state/open-questions.md": "Q",
    "docs/agents/state/task-board.md": "TASK",
}


def headings(text, prefix):
    return re.findall(rf"^### ({prefix}-\d+)\b", text, re.MULTILINE)


def index_rows(text, prefix):
    return re.findall(rf"^\| ({prefix}-\d+) \|", text, re.MULTILINE)


def main(root):
    errors = []
    for rel, prefix in RECORDS.items():
        path = root / rel
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        ids = headings(text, prefix)
        for dup, count in collections.Counter(ids).items():
            if count > 1:
                errors.append(f"{rel}: {dup} is used by {count} entries; renumber the later one")
        if prefix in ("DEC", "PRB"):
            rows = index_rows(text, prefix)
            for dup, count in collections.Counter(rows).items():
                if count > 1:
                    errors.append(f"{rel}: {dup} has {count} index rows")
            if rows:
                for missing in sorted(set(ids) - set(rows)):
                    errors.append(f"{rel}: {missing} has no index row")
                for orphan in sorted(set(rows) - set(ids)):
                    errors.append(f"{rel}: index row {orphan} has no entry")
    for error in errors:
        print(f"::error::{error}")
    if not errors:
        print("Record ids are unique and the indexes match.")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".")))
