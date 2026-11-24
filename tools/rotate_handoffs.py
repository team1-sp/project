"""Move old handoff entries from the live log to monthly archive files (DEC-152.1).

The live file docs/agents/state/handoff-log.md keeps only the newest KEEP entries, so an agent
that loads the whole file at boot reads a small, bounded file. Older entries move, unchanged,
to docs/history/handoff-archive/<YYYY-MM>.md by the month of their **Date** line, appended in
log order. Entries are never edited, only moved.

Run it in a docs PR when tools/check_ids.py reports that the live log has more than LIMIT
entries (it says so), or at the end of an increment:

    python tools/rotate_handoffs.py [repo root]

Two branches that rotate at the same time make the same change, so git merges them cleanly;
tools/check_ids.py fails if an entry ends up in two files.
"""
import pathlib
import re
import sys

LIVE = "docs/agents/state/handoff-log.md"
ARCHIVE = "docs/history/handoff-archive"
KEEP = 10   # entries left in the live log after a rotation
LIMIT = 15  # tools/check_ids.py fails above this many live entries

ENTRY = re.compile(r"(?m)^(?=### HND-)")
DATE = re.compile(r"^- \*\*Date:\*\* (\d{4}-\d{2})", re.MULTILINE)


def split(text):
    """Return (preamble, [entry, ...]); each entry keeps its own trailing `---` separator."""
    parts = ENTRY.split(text)
    return parts[0], parts[1:]


def archive_header(month):
    return (f"# Handoff Archive {month}\n\n"
            "**Moved, not edited.** Entries older than the newest ten, moved out of the live "
            f"[handoff log](../../agents/state/handoff-log.md) by `tools/rotate_handoffs.py`. "
            "Agents read this only when they trace earlier work back.\n\n---\n\n")


def rotate(root):
    live = root / LIVE
    preamble, entries = split(live.read_text(encoding="utf-8"))
    if len(entries) <= KEEP:
        return 0
    old, keep = entries[:-KEEP], entries[-KEEP:]
    by_month = {}
    for entry in old:
        match = DATE.search(entry)
        if not match:
            sys.exit(f"{LIVE}: entry without a '- **Date:** YYYY-MM-DD' line:\n{entry[:200]}")
        by_month.setdefault(match.group(1), []).append(entry)
    folder = root / ARCHIVE
    folder.mkdir(parents=True, exist_ok=True)
    for month, moved in by_month.items():
        path = folder / f"{month}.md"
        text = path.read_text(encoding="utf-8") if path.exists() else archive_header(month)
        if not text.endswith("\n"):
            text += "\n"
        path.write_text(text + "".join(moved), encoding="utf-8")
    live.write_text(preamble + "".join(keep), encoding="utf-8")
    return len(old)


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    root = pathlib.Path(argv[0] if argv else ".")
    moved = rotate(root)
    print(f"moved {moved} handoff entries to {ARCHIVE}/" if moved else "nothing to rotate")


if __name__ == "__main__":
    main()
