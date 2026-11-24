"""Renumber the requirements so their ids run 1..n without gaps, and fix every reference (DEC-152.3).

The SRS numbers requirements from 1 without gaps in each group. Instead of renumbering by hand,
edit docs/requirements/ freely and run this script; it needs no AI:

- **Add** a requirement: put its line where it belongs, with any placeholder id in its group,
  for example `- **srs.fr.14.new** ...` (or `srs.fr.14.new2` for a second one), plus its
  `  - Use cases: ...` line. Reference it elsewhere by the same placeholder if needed.
- **Remove** a requirement: delete its two lines. References to it are reported, not guessed.
- **Move** one to another group: move its lines to that file; it gets that group's next number.

Then run `python tools/renumber_requirements.py`. It numbers every functional group
(srs.fr.<n>.*) in file order and every non-functional group (srs.nfr.<n>.*) in file order,
rewrites each changed id in one pass across all tracked text files (so 14.15 -> 14.16 never
chains into 14.17), and appends the old -> new table to docs/requirements/renumbering.md.
History is never rewritten: docs/history/, the handoff log and docs/research/ keep the ids they
were written with, and renumbering.md lets a reader trace them. `srs.fr.0.*` is an example id.

Pending change files (docs/requirements/changes/, DEC-153.1) are rewritten too, so their target
ids stay right; feature work writes those files and tools/apply_requirement_changes.py applies
them and then calls this script.

It stops without writing anything when a removed id is still referenced outside history, and
lists where. `--check` only reports whether a renumbering is needed (CI runs it).

Usage: python tools/renumber_requirements.py [--check] [repo root]
"""
import datetime
import pathlib
import re
import subprocess
import sys

REQ = "docs/requirements"
LOG = f"{REQ}/renumbering.md"
SKIP = ("docs/history/", "docs/agents/state/handoff-log.md", "docs/research/", "tools/renumber_requirements.py", f"{REQ}/README.md", LOG,
        "tools/apply_requirement_changes.py")  # history and human-only notes; pending change files ARE rewritten
TEXT = {".md", ".cpp", ".hpp", ".h", ".c", ".py", ".proto", ".txt", ".cmake", ".json", ".yml", ".yaml", ".toml",
        ".ini", ".cfg"}
ID = re.compile(r"\bsrs\.(n?fr)\.(\d+)\.(\w+)\b")
FR_LINE = re.compile(r"^- \*\*srs\.fr\.(\d+)\.(\w+)\*\* ")
NFR_LINE = re.compile(r"^(- \*\*|\| )srs\.nfr\.(\d+)\.(\w+)(\*\* | \|)")
GROUP = re.compile(r"^group: srs\.fr\.(\d+)$", re.MULTILINE)


def tracked(root):
    out = subprocess.run(["git", "-C", str(root), "ls-files"], capture_output=True, text=True, check=True).stdout
    for rel in out.splitlines():
        if pathlib.PurePosixPath(rel).suffix in TEXT and (root / rel).is_file():
            yield rel


def plan(root):
    """Return (mapping old id -> new id for every requirement line, errors)."""
    mapping, errors = {}, []
    for path in sorted((root / REQ / "functional").glob("ep-*.md")):
        text = path.read_text(encoding="utf-8")
        group = int(GROUP.search(text).group(1))
        number = 0
        for line in text.split("\n"):
            match = FR_LINE.match(line)
            if match:
                number += 1
                old = f"srs.fr.{match.group(1)}.{match.group(2)}"
                if old in mapping:
                    errors.append(f"{old} appears twice in {REQ}/functional; give the new one a placeholder like srs.fr.{group}.new")
                mapping[old] = f"srs.fr.{group}.{number}"
    counters = {}
    for path in sorted((root / REQ / "non-functional").glob("*.md")):
        for line in path.read_text(encoding="utf-8").split("\n"):
            match = NFR_LINE.match(line)
            if match:
                group = int(match.group(2))
                counters[group] = counters.get(group, 0) + 1
                old = f"srs.nfr.{group}.{match.group(3)}"
                if old in mapping:
                    errors.append(f"{old} appears twice in {REQ}/non-functional")
                mapping[old] = f"srs.nfr.{group}.{counters[group]}"
    return mapping, errors


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    check = "--check" in argv
    rest = [a for a in argv if a != "--check"]
    root = pathlib.Path(rest[0] if rest else ".").resolve()
    mapping, errors = plan(root)
    changed = {old: new for old, new in mapping.items() if old != new}

    # Ids before the edit that no longer exist: removed requirements (or typos) still referenced.
    files = [rel for rel in tracked(root) if not rel.startswith(SKIP)]
    dangling = []
    texts = {}
    for rel in files:
        text = (root / rel).read_text(encoding="utf-8", errors="surrogateescape")
        texts[rel] = text
        for number, line in enumerate(text.split("\n"), 1):
            for match in ID.finditer(line):
                if match.group(0) not in mapping and match.group(2) != "0":  # srs.fr.0.0 = template example
                    dangling.append(f"{rel}:{number}: {match.group(0)} is not a requirement (removed, or a typo)")
    errors += dangling

    if check:
        for error in errors:
            print(f"::error::{error}")
        if changed:
            print(f"::error::{len(changed)} requirement id(s) are out of order or placeholders "
                  f"(e.g. {next(iter(changed))} -> {next(iter(changed.values()))}); run `python tools/renumber_requirements.py`")
        if not errors and not changed:
            print("Requirement ids run 1..n without gaps and every reference resolves.")
        return 1 if errors or changed else 0

    if errors:
        for error in errors:
            print(f"error: {error}")
        print("Nothing was changed. Fix the references above (point them at the replacing requirement), then run again.")
        return 1
    if not changed:
        print("Nothing to renumber.")
        return 0

    def swap(match):
        return changed.get(match.group(0), match.group(0))

    touched = []
    for rel, text in texts.items():
        new = ID.sub(swap, text)
        if new != text:
            (root / rel).write_text(new, encoding="utf-8", errors="surrogateescape")
            touched.append(rel)
    log = root / LOG
    head = log.read_text(encoding="utf-8") if log.exists() else "# Requirement Renumbering\n"
    rows = "\n".join(f"| {old} | {new} |" for old, new in changed.items())
    log.write_text(head.rstrip("\n") + f"\n\n## {datetime.date.today().isoformat()}\n\n| Old | New |\n| --- | --- |\n{rows}\n",
                   encoding="utf-8")
    print(f"renumbered {len(changed)} id(s) in {len(touched)} file(s); table added to {LOG}")
    return 0



if __name__ == "__main__":
    sys.exit(main())
