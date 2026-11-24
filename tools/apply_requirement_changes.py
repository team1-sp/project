"""Apply the pending requirement changes in docs/requirements/changes/, then renumber (DEC-<n>.1).

Feature work does not edit the epic files or renumber ids. It writes what should change in
`docs/requirements/changes/<n>.md` (`<n>` = the issue the pull request closes, or its own number
for no-issue work) and cites new requirements by their change id `RC-<n>.<k>` in code, tests and
docs. When the team wants the requirement files up to date (before a docs sync, at the latest
before a deliverable is built), one pull request runs this script. It:

1. checks every pending file (targets exist, no requirement is changed by two files);
2. applies them in file order: adds, rewords, removes and moves;
3. points every reference to a removed requirement at its replacement;
4. rewrites every `RC-<n>.<k>` across the repository to a placeholder of its group, and runs
   tools/renumber_requirements.py, which numbers each group 1..n and rewrites every reference;
5. deletes the applied files and lists them in docs/requirements/renumbering.md.

Nothing is written when a check fails. `--check` only validates the pending files and that every
`RC-` id cited in the repository has a pending definition (CI runs it).

A change file (see docs/requirements/changes/README.md for the full form):

    # Requirement changes: #171

    Decided by: DEC-171.1

    ## Add after srs.fr.14.15
    - **RC-171.1** When the user ..., the Editor shall ...
      - Use cases: UC-03; Initial Plan: —

    ## Add at end of US-14.2
    ## Add at end of non-functional/constraints.md
    ## Reword srs.fr.2.17          (the new lines follow, with the requirement's own id)
    ## Remove srs.fr.3.4           (then `References: srs.fr.3.5` or `References: none`)
    ## Move srs.fr.5.2 after srs.fr.7.3

Usage: python tools/apply_requirement_changes.py [--check] [repo root]
"""
import pathlib
import re
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import renumber_requirements as renumber  # noqa: E402

CHANGES = f"{renumber.REQ}/changes"
RC = re.compile(r"\bRC-(\d+)\.(\d+)\b")
ANY_ID = r"(?:srs\.n?fr\.\d+\.\w+|RC-\d+\.\d+)"
OP = re.compile(rf"^## (?:(Add) (?:after ({ANY_ID})|at end of (US-\d+\.\d+|non-functional/[\w-]+\.md))"
                rf"|(Reword) (srs\.n?fr\.\d+\.\w+)|(Remove) (srs\.n?fr\.\d+\.\w+)"
                rf"|(Move) (srs\.n?fr\.\d+\.\w+) after ({ANY_ID}))\s*$")
ENTRY = re.compile(rf"^(?:- \*\*|\| )({ANY_ID})(?:\*\* | \|)")
MAP = re.compile(r"^  - Use cases: ")
REFS = re.compile(rf"^References: (none|{ANY_ID})\s*$")
SKIP = renumber.SKIP + (f"{CHANGES}/README.md",)


class Error(Exception):
    pass


def pending(root):
    folder = root / CHANGES
    files = [p for p in folder.glob("*.md") if p.name != "README.md"] if folder.is_dir() else []
    return sorted(files, key=lambda p: int(p.stem) if p.stem.isdigit() else 10 ** 9)


def parse(path):
    """Return the operations of one change file as dicts, in file order."""
    if not path.stem.isdigit():
        raise Error(f"{path.name}: name it <issue or PR number>.md")
    ops, current = [], None
    for number, line in enumerate(path.read_text(encoding="utf-8").split("\n"), 1):
        where = f"{path.name}:{number}"
        if line.startswith("## "):
            m = OP.match(line)
            if not m:
                raise Error(f"{where}: unknown heading {line!r} (Add after / Add at end of / Reword / "
                            "Remove / Move ... after)")
            g = m.groups()
            if g[0]:
                current = {"op": "add", "after": g[1], "end": g[2]}
            elif g[3]:
                current = {"op": "reword", "id": g[4]}
            elif g[5]:
                current = {"op": "remove", "id": g[6], "refs": None}
            else:
                current = {"op": "move", "id": g[8], "after": g[9]}
            current.update(lines=[], where=where)
            ops.append(current)
        elif current is None or not line.strip():
            continue
        elif current["op"] == "remove":
            m = REFS.match(line)
            if not m:
                raise Error(f"{where}: after Remove write 'References: <replacing id>' or 'References: none'")
            current["refs"] = m.group(1)
        elif current["op"] == "move":
            raise Error(f"{where}: Move takes no lines")
        else:
            current["lines"].append(line)
    for op in ops:
        if op["op"] == "remove" and op["refs"] is None:
            raise Error(f"{op['where']}: Remove needs 'References: <replacing id>' or 'References: none'")
        if op["op"] in ("add", "reword"):
            entries = [ENTRY.match(x) for x in op["lines"]]
            ids = [e.group(1) for e in entries if e]
            if not ids:
                raise Error(f"{op['where']}: no requirement line under this heading")
            for rid in ids:
                if op["op"] == "add" and not re.fullmatch(rf"RC-{path.stem}\.\d+", rid):
                    raise Error(f"{op['where']}: a new requirement is written RC-{path.stem}.<k>, not {rid}")
                if op["op"] == "reword" and rid != op["id"]:
                    raise Error(f"{op['where']}: Reword {op['id']} must keep the id {op['id']}, not {rid}")
            op["ids"] = ids
    return ops


class Requirements:
    """The requirement files as lists of lines, with lookups by id."""

    def __init__(self, root):
        self.root = root
        self.files = {}
        for path in sorted((root / renumber.REQ / "functional").glob("ep-*.md")) + sorted(
                (root / renumber.REQ / "non-functional").glob("*.md")):
            self.files[path.relative_to(root).as_posix()] = path.read_text(encoding="utf-8").split("\n")

    def find(self, rid):
        """(file, first line, number of lines) of a requirement or change id."""
        for rel, lines in self.files.items():
            for i, line in enumerate(lines):
                m = ENTRY.match(line)
                if m and m.group(1) == rid:
                    n = 2 if i + 1 < len(lines) and MAP.match(lines[i + 1]) else 1
                    return rel, i, n
        raise Error(f"{rid} is not in the requirement files")

    def story_end(self, story):
        for rel, lines in self.files.items():
            for i, line in enumerate(lines):
                if line.startswith(f"## {story} "):
                    j = i + 1
                    while j < len(lines) and not lines[j].startswith("## "):
                        j += 1
                    while j > i + 1 and not lines[j - 1].strip():
                        j -= 1
                    return rel, j
        raise Error(f"{story} is not a story heading in the requirement files")

    def file_end(self, rel_name):
        rel = f"{renumber.REQ}/{rel_name}"
        if rel not in self.files:
            raise Error(f"{rel_name} is not a requirement file")
        lines = self.files[rel]
        j = len(lines)
        while j > 0 and not lines[j - 1].strip():
            j -= 1
        return rel, j

    def group_of(self, rel):
        text = "\n".join(self.files[rel])
        m = renumber.GROUP.search(text)
        if m:
            return f"srs.fr.{m.group(1)}"
        m = re.search(r"srs\.nfr\.(\d+)\.", text)
        if not m:
            raise Error(f"{rel}: cannot tell its group; add after an existing requirement instead")
        return f"srs.nfr.{m.group(1)}"


def plan_and_apply(root, files):
    """Apply every change in memory. Return (requirement files, rc -> placeholder, removed -> repl)."""
    req = Requirements(root)
    touched, rc_place, removed = {}, {}, {}
    for path in files:
        for op in parse(path):
            where = op["where"]
            try:
                if op["op"] in ("reword", "remove", "move"):
                    if op["id"] in touched:
                        raise Error(f"{op['id']} is already changed by {touched[op['id']]}")
                    touched[op["id"]] = where
                if op["op"] == "reword":
                    rel, i, n = req.find(op["id"])
                    new = list(op["lines"])
                    if n == 2 and len(new) == 1:
                        new.append(req.files[rel][i + 1])      # keep the Use cases line
                    req.files[rel][i:i + n] = new
                elif op["op"] == "remove":
                    rel, i, n = req.find(op["id"])
                    del req.files[rel][i:i + n]
                    removed[op["id"]] = op["refs"]
                else:
                    if op["op"] == "move":
                        rel, i, n = req.find(op["id"])
                        block = req.files[rel][i:i + n]
                        del req.files[rel][i:i + n]
                    else:
                        block = list(op["lines"])
                    if op.get("after"):
                        rel, i, n = req.find(op["after"])
                        at = i + n
                    elif op["end"].startswith("US-"):
                        rel, at = req.story_end(op["end"])
                    else:
                        rel, at = req.file_end(op["end"])
                    group = req.group_of(rel)
                    if op["op"] == "add":
                        for rid in op["ids"]:
                            if rid in rc_place:
                                raise Error(f"{rid} is added twice")
                            rc_place[rid] = f"{group}.rc{rid[3:].replace('.', 'x')}"
                    req.files[rel][at:at] = block
            except Error as exc:
                raise Error(f"{where}: {exc}") from None
    # The new lines carry their change ids; give them placeholders of their group.
    for rel, lines in req.files.items():
        req.files[rel] = [RC.sub(lambda m: rc_place.get(m.group(0), m.group(0)), x)
                          if ENTRY.match(x) else x for x in lines]
    for old, repl in removed.items():
        if repl in removed:
            raise Error(f"{old} is replaced by {repl}, which is removed too")
    return req, rc_place, removed


def rc_citations(root, defined):
    out = []
    for rel in renumber.tracked(root):
        if rel.startswith(SKIP):
            continue
        for number, line in enumerate((root / rel).read_text(encoding="utf-8", errors="surrogateescape")
                                      .split("\n"), 1):
            for m in RC.finditer(line):
                if m.group(0) not in defined:
                    out.append(f"{rel}:{number}: {m.group(0)} has no pending change file "
                               "(applied already? use the final id from renumbering.md)")
    return out


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    check = "--check" in argv
    rest = [a for a in argv if a != "--check"]
    root = pathlib.Path(rest[0] if rest else ".").resolve()
    files = pending(root)
    try:
        req, rc_place, removed = plan_and_apply(root, files)
    except Error as exc:
        print(f"{'::error::' if check else 'error: '}{exc}")
        print("Nothing was changed.")
        return 1
    stale = rc_citations(root, rc_place)
    if check:
        for line in stale:
            print(f"::error::{line}")
        if not stale:
            print(f"{len(files)} pending change file(s), {len(rc_place)} new requirement(s); all valid.")
        return 1 if stale else 0
    if stale:
        for line in stale:
            print(f"error: {line}")
        print("Nothing was changed.")
        return 1
    if not files:
        print("No pending requirement changes.")
        return 0

    # Write, run the renumbering, and roll back if it refuses.
    originals = {}

    def write(rel, text):
        path = root / rel
        if rel not in originals:
            originals[rel] = path.read_text(encoding="utf-8", errors="surrogateescape")
        path.write_text(text, encoding="utf-8", errors="surrogateescape")

    swap = dict(rc_place)
    swap.update({old: (rc_place.get(repl, repl)) for old, repl in removed.items() if repl != "none"})
    pattern = re.compile(r"\bRC-\d+\.\d+\b|\bsrs\.n?fr\.\d+\.\w+\b")

    def fix(text):
        return pattern.sub(lambda m: swap.get(m.group(0), m.group(0)), text)

    for rel, lines in req.files.items():
        write(rel, fix("\n".join(lines)))
    for rel in renumber.tracked(root):
        if rel.startswith(SKIP) or rel in req.files:
            continue
        text = (root / rel).read_text(encoding="utf-8", errors="surrogateescape")
        new = fix(text)
        if new != text:
            write(rel, new)
    change_texts = {p: p.read_text(encoding="utf-8") for p in files}
    for p in files:
        p.unlink()
    if renumber.main([str(root)]) != 0:
        for rel, text in originals.items():
            (root / rel).write_text(text, encoding="utf-8", errors="surrogateescape")
        for p, text in change_texts.items():
            p.write_text(text, encoding="utf-8")
        print("Rolled back: nothing was changed. A removed requirement is still cited (listed "
              "above): name its replacement in the change file ('References: <id>').")
        return 1
    log = root / renumber.LOG
    names = ", ".join(f"#{p.stem}" for p in files)
    log.write_text(log.read_text(encoding="utf-8").rstrip("\n") + f"\n\nApplied change files: {names}.\n",
                   encoding="utf-8")
    print(f"applied {len(files)} change file(s): {names}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
