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


def compare_authors(authors, creators):
    """Match authors by name, so that a missing or extra author is reported
    directly instead of shifting all subsequent positional comparisons."""
    errors = []

    if len(authors) != len(creators):
        errors.append(f"number of authors differs: CITATION.cff has {len(authors)}, "
                      f".zenodo.json has {len(creators)}")

    names_a = [a["name"] for a in authors]
    names_c = [c["name"] for c in creators]
    for label, names in (("CITATION.cff", names_a), (".zenodo.json", names_c)):
        dups = sorted({n for n in names if names.count(n) > 1})
        for n in dups:
            errors.append(f"'{n}' is listed more than once in {label}")

    by_name_a = {a["name"]: a for a in authors}
    by_name_c = {c["name"]: c for c in creators}

    for name in names_a:
        if name not in by_name_c:
            errors.append(f"'{name}' is in CITATION.cff but missing in .zenodo.json")
    for name in names_c:
        if name not in by_name_a:
            errors.append(f"'{name}' is in .zenodo.json but missing in CITATION.cff")

    for name in names_a:
        if name in by_name_c and by_name_a[name] != by_name_c[name]:
            errors.append(f"'{name}' differs:\n"
                          f"    CITATION.cff: {by_name_a[name]}\n"
                          f"    .zenodo.json: {by_name_c[name]}")

    # the order is only meaningful once both files list the same people
    if not errors and names_a != names_c:
        errors.append("author order differs:\n"
                      f"    CITATION.cff: {names_a}\n"
                      f"    .zenodo.json: {names_c}")

    return errors


def main():
    root = Path(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parents[2])
    cff = yaml.safe_load((root / "CITATION.cff").read_text(encoding="utf-8"))
    zenodo = json.loads((root / ".zenodo.json").read_text(encoding="utf-8"))
    cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")

    errors = []

    authors = cff_authors(cff)
    creators = zenodo_creators(zenodo)
    errors.extend(compare_authors(authors, creators))

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
