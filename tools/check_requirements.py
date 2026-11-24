"""Check that docs/requirements/ keeps the fixed format the docs repository reads (DEC-152.2).

The course deliverables (SRS sections 6.1, 6.3, 6.4 and 7.x) are rebuilt from these files by a
script in team1-sp/docs, without AI. That only works while every file keeps this shape:

functional/ep-<nn>-<slug>.md
    front matter with `epic: EP-<nn>`, `group: srs.fr.<n>`, `part: engine|editor` and an
    optional `epic_title:`; then `# EP-<nn> <group title>`, `**Epic:** ...`, optional intro
    paragraphs, and one `## US-<n>.<k> <subsection title>` per story with `**Story:** ...`,
    optional `*Screens: ...*` and the requirements, each one line
    `- **srs.fr.<n>.<m>** <text>` followed by `  - Use cases: <...>; Initial Plan: <...>`.
non-functional/<name>.md
    `# <title>`, optional `## <subsection>` headings, and `srs.nfr.<n>.<m>` items as list lines
    or table rows.

Ids are numbered from 1 without gaps in each group, in file order. tools/renumber_requirements.py
restores that after an edit (DEC-152.3).

Usage: python tools/check_requirements.py [repo root]
"""
import pathlib
import re
import sys

REQ = "docs/requirements"
FRONT = re.compile(r"\A---\n(.*?)\n---\n\n", re.DOTALL)
FR = re.compile(r"^- \*\*srs\.fr\.(\d+)\.(\d+)\*\* \S")
MAP = re.compile(r"^  - Use cases: [^;]+; Initial Plan: \S.*$")
NFR = re.compile(r"(?:^- \*\*|^\| )srs\.nfr\.(\d+)\.(\d+)(?:\*\* | \|)")
STORY = re.compile(r"^## US-(\d+)\.(\d+) \S")
ALLOWED = (re.compile(r"^$"), re.compile(r"^\*\*Story:\*\* \S"), re.compile(r"^\*Screens: .+\*$"))


def check_functional(path, errors):
    rel = path.as_posix()
    text = path.read_text(encoding="utf-8")
    front = FRONT.match(text)
    if not front:
        errors.append(f"{rel}: missing front matter")
        return
    meta = dict(line.split(": ", 1) for line in front.group(1).splitlines() if ": " in line)
    match = re.fullmatch(r"EP-(\d{2})", meta.get("epic", ""))
    if not match:
        errors.append(f"{rel}: front matter needs `epic: EP-<nn>`")
        return
    group = int(match.group(1))
    if meta.get("group") != f"srs.fr.{group}":
        errors.append(f"{rel}: `group` must be srs.fr.{group}")
    if meta.get("part") not in ("engine", "editor"):
        errors.append(f"{rel}: `part` must be engine or editor")
    lines = text[front.end():].split("\n")
    if not re.match(rf"^# EP-{group:02d} \S", lines[0]):
        errors.append(f"{rel}:1: title must be `# EP-{group:02d} <group title>`")
    if not any(line.startswith("**Epic:** ") for line in lines):
        errors.append(f"{rel}: missing `**Epic:** ...` line")
    story = 0
    expected_fr = 1
    has_story_text = True
    frs_in_story = 0
    previous_fr = False
    for number, line in enumerate(lines, start=front.group(0).count("\n") + 1):
        where = f"{rel}:{number}"
        match = STORY.match(line)
        if match:
            if story and (not has_story_text or not frs_in_story):
                errors.append(f"{where}: the story before this one needs a `**Story:**` line and requirements")
            story += 1
            if (int(match.group(1)), int(match.group(2))) != (group, story):
                errors.append(f"{where}: expected `## US-{group}.{story} ...`")
            has_story_text, frs_in_story = False, 0
            continue
        if not story:
            continue
        if line.startswith("**Story:** "):
            has_story_text = True
        match = FR.match(line)
        if match:
            if previous_fr:
                errors.append(f"{where}: the requirement above needs a `  - Use cases: ...; Initial Plan: ...` line")
            if (int(match.group(1)), int(match.group(2))) != (group, expected_fr):
                errors.append(f"{where}: expected srs.fr.{group}.{expected_fr} (ids run from 1 without gaps; run `python tools/renumber_requirements.py`)")
            expected_fr = int(match.group(2)) + 1
            frs_in_story += 1
            previous_fr = True
            continue
        if MAP.match(line):
            if not previous_fr:
                errors.append(f"{where}: a `Use cases` line must follow its requirement")
            previous_fr = False
            continue
        if previous_fr:
            errors.append(f"{where}: the requirement above needs a `  - Use cases: ...; Initial Plan: ...` line")
            previous_fr = False
        if not any(p.match(line) for p in ALLOWED):
            errors.append(f"{where}: unexpected line inside a story (requirements are one line each): {line[:60]}")
    if previous_fr:
        errors.append(f"{rel}: the last requirement needs its `Use cases` line")
    if not story:
        errors.append(f"{rel}: no `## US-` stories")
    elif not has_story_text or not frs_in_story:
        errors.append(f"{rel}: the last story needs a `**Story:**` line and requirements")


def check_nonfunctional(paths, errors):
    seen = {}
    for path in paths:
        lines = path.read_text(encoding="utf-8").split("\n")
        if not lines[0].startswith("# "):
            errors.append(f"{path.as_posix()}:1: title must be `# <title>`")
        for number, line in enumerate(lines, 1):
            match = NFR.match(line)
            if match:
                key = (int(match.group(1)), int(match.group(2)))
                if key in seen:
                    errors.append(f"{path.as_posix()}:{number}: srs.nfr.{key[0]}.{key[1]} also in {seen[key]}")
                seen[key] = path.as_posix()
    groups = {}
    for group, item in seen:
        groups.setdefault(group, []).append(item)
    for group, items in groups.items():
        if sorted(items) != list(range(1, len(items) + 1)):
            errors.append(f"srs.nfr.{group}.*: ids must run from 1 without gaps, found {sorted(items)}; run `python tools/renumber_requirements.py`")


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    root = pathlib.Path(argv[0] if argv else ".")
    req = root / REQ
    errors = []
    epics = sorted((req / "functional").glob("ep-*.md"))
    for path in epics:
        check_functional(path, errors)
    numbers = sorted(int(re.match(r"ep-(\d+)", p.name).group(1)) for p in epics)
    if numbers != list(range(1, len(numbers) + 1)):
        errors.append(f"{REQ}/functional: epics must run EP-01 to EP-{len(numbers):02d} without gaps, found {numbers}")
    check_nonfunctional(sorted((req / "non-functional").glob("*.md")), errors)
    if not (req / "conventions.md").exists():
        errors.append(f"{REQ}/conventions.md is missing")
    for error in errors:
        print(f"::error::{error}")
    if not errors:
        print(f"Requirements format is valid: {len(epics)} epics.")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
