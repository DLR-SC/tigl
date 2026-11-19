#!/usr/bin/env python3
"""Check that CITATION.cff and .zenodo.json are consistent.

CITATION.cff is the source of truth. The check fails if
  - the authors in CITATION.cff and the creators in .zenodo.json differ
    in name, affiliation, ORCID or order, or
  - the version in CITATION.cff differs from TIGL_VERSION in CMakeLists.txt.

Usage: python check_citation_sync.py [repo_root]
"""

import json
import re
import sys
from pathlib import Path

import yaml


def cff_authors(cff):
    result = []
    for a in cff.get("authors", []):
        orcid = a.get("orcid", "")
        result.append({
            "name": f"{a['family-names']}, {a['given-names']}",
            "affiliation": a.get("affiliation", ""),
            "orcid": orcid.replace("https://orcid.org/", ""),
        })
    return result


def zenodo_creators(zenodo):
    return [{
        "name": c["name"],
        "affiliation": c.get("affiliation", ""),
        "orcid": c.get("orcid", ""),
    } for c in zenodo.get("creators", [])]


def main():
    root = Path(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parents[2])
    cff = yaml.safe_load((root / "CITATION.cff").read_text(encoding="utf-8"))
    zenodo = json.loads((root / ".zenodo.json").read_text(encoding="utf-8"))
    cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")

    errors = []

    authors = cff_authors(cff)
    creators = zenodo_creators(zenodo)
    for i in range(max(len(authors), len(creators))):
        a = authors[i] if i < len(authors) else None
        c = creators[i] if i < len(creators) else None
        if a != c:
            errors.append(f"author #{i + 1} differs:\n"
                          f"    CITATION.cff: {a}\n"
                          f"    .zenodo.json: {c}")

    match = re.search(r"set\s*\(\s*TIGL_VERSION\s+([0-9][^\s)]*)", cmake)
    if not match:
        errors.append("could not find set(TIGL_VERSION ...) in CMakeLists.txt")
    elif str(cff.get("version")) != match.group(1):
        errors.append(f"version differs: CITATION.cff has {cff.get('version')}, "
                      f"CMakeLists.txt has {match.group(1)}")

    if errors:
        print("CITATION.cff and .zenodo.json / CMakeLists.txt are out of sync:")
        for e in errors:
            print("  - " + e)
        return 1

    print(f"OK: {len(authors)} authors in sync, version {cff.get('version')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
