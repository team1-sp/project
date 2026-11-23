"""Keep the files every agent reads at session start under a size budget (DEC-152.1).

The boot set is listed in docs/agents/README.md ("Reading tiers") and here. Every agent session
pays for these files in tokens, so they must stay small: decisions are read through their
one-line index, closed questions and old handoffs move to docs/history/, and problems are read
only when a problem occurs. The live handoff log is left out of the budget because
tools/check_ids.py already bounds it by entry count.

Usage: python tools/check_boot.py [repo root]
"""
import pathlib
import sys

BOOT = [
    "docs/agents/README.md",
    "docs/agents/AGENT-PROTOCOL.md",
    "docs/agents/rules.md",
    "docs/agents/state/current-status.md",
    "docs/agents/state/open-questions.md",
    "docs/history/decision-index.md",
]
BUDGET = 50 * 1024  # bytes, for the files above together


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    root = pathlib.Path(argv[0] if argv else ".")
    missing = [rel for rel in BOOT if not (root / rel).exists()]
    if missing:
        for rel in missing:
            print(f"::error::boot file {rel} is missing; update BOOT in tools/check_boot.py and docs/agents/README.md")
        return 1
    sizes = {rel: (root / rel).stat().st_size for rel in BOOT}
    total = sum(sizes.values())
    for rel, size in sorted(sizes.items(), key=lambda item: -item[1]):
        print(f"{size / 1024:6.1f} KB  {rel}")
    print(f"{total / 1024:6.1f} KB  total (budget {BUDGET / 1024:.0f} KB)")
    if total > BUDGET:
        print(f"::error::the agent boot files are {total / 1024:.1f} KB, over the {BUDGET / 1024:.0f} KB budget. "
              "Move closed or detailed content out of them (archive files, playbooks) instead of raising the budget.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
