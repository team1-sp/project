"""Fail if a Markdown file links to a repo path that does not exist.

Only relative links are checked; web links and in-page anchors are skipped.
Usage: python tools/check_md_links.py [root]
"""
import pathlib
import re
import sys

LINK = re.compile(r"\[[^\]]*\]\(([^)\s]+)(?:\s+\"[^\"]*\")?\)")
SKIP_DIRS = {".git", "build", "out", "node_modules", ".venv"}


def main():
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    broken = []
    for md in root.rglob("*.md"):
        if SKIP_DIRS & set(md.relative_to(root).parts):
            continue
        text = md.read_text(encoding="utf-8")
        # Ignore links inside fenced code blocks.
        text = re.sub(r"```.*?```", "", text, flags=re.DOTALL)
        for target in LINK.findall(text):
            if re.match(r"^[a-z][a-z0-9+.-]*:", target, re.IGNORECASE) or target.startswith("#"):
                continue
            path = target.split("#", 1)[0]
            if not path:
                continue
            resolved = (root / path.lstrip("/")) if path.startswith("/") else (md.parent / path)
            if not resolved.exists():
                broken.append(f"{md.relative_to(root)}: {target}")
    for item in broken:
        print(f"::error::Broken link in {item}")
    print(f"{len(broken)} broken link(s).")
    sys.exit(1 if broken else 0)


if __name__ == "__main__":
    main()
