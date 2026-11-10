"""Fail if a library header is included outside the target that may use it (R-3, R-4).

Box2D may only be included in engine/physics-box2d/ and raylib only in
engine/rendering-raylib/. Everything else in engine/ and editor/ must go through our own
interfaces. sandbox/ is not checked: it holds the PB-008 spike (DEC-037).
Usage: python tools/check_includes.py [root]
"""
import pathlib
import re
import sys

CHECKED_DIRS = ["engine", "editor"]
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl"}
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.MULTILINE)

# (library, header pattern, the only folder allowed to include it)
RULES = [
    ("Box2D", re.compile(r"^box2d/"), "engine/physics-box2d/"),
    ("raylib", re.compile(r"^(raylib|raymath|rlgl)\.h$"), "engine/rendering-raylib/"),
]


def main():
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    problems = []
    for folder in CHECKED_DIRS:
        for path in sorted((root / folder).rglob("*")):
            if path.suffix not in SOURCE_SUFFIXES or not path.is_file():
                continue
            relative = path.relative_to(root).as_posix()
            text = path.read_text(encoding="utf-8", errors="replace")
            for header in INCLUDE.findall(text):
                for library, pattern, allowed in RULES:
                    if pattern.search(header) and not relative.startswith(allowed):
                        problems.append(f"{relative}: includes <{header}>; {library} is only "
                                        f"allowed in {allowed}")
    for problem in problems:
        print(problem)
    print(f"{len(problems)} include rule violation(s).")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
