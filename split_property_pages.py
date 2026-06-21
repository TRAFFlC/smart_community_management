#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import re
from pathlib import Path

ROOT = Path(r"d:\smart_community_management")
SRC = ROOT / "src" / "pages"
ORIGIN = SRC / "buildPropertySubPages.cpp"
PROPERTY_PAGE = SRC / "PropertyPage.cpp"

NAMES = [
    "Workorder",
    "Complaint",
    "Inspection",
    "Announcement",
    "Visitor",
    "Topic",
    "Parking",
    "Billing",
    "Income",
]


def extract_function(text: str, prefix: str) -> str:
    """Extract a function starting with `prefix` using brace counting, skipping strings/comments."""
    start = text.find(prefix)
    if start < 0:
        raise ValueError(f"Function prefix not found: {prefix}")
    # find opening brace on the next line after signature
    brace_start = text.find("{", text.find("\n", start))
    if brace_start < 0:
        raise ValueError(f"Opening brace not found for {prefix}")

    depth = 1
    i = brace_start + 1
    state = "none"  # none, line_comment, block_comment, string, char
    while i < len(text) and depth > 0:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if state == "none":
            if ch == "/" and nxt == "/":
                state = "line_comment"
                i += 2
                continue
            if ch == "/" and nxt == "*":
                state = "block_comment"
                i += 2
                continue
            if ch == '"':
                state = "string"
                i += 1
                continue
            if ch == "'":
                state = "char"
                i += 1
                continue
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
        elif state == "line_comment":
            if ch == "\n":
                state = "none"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "none"
                i += 2
                continue
        elif state == "string":
            if ch == "\\":
                i += 2
                continue
            elif ch == '"':
                state = "none"
        elif state == "char":
            if ch == "\\":
                i += 2
                continue
            elif ch == "'":
                state = "none"
        i += 1

    if depth != 0:
        raise ValueError(f"Unbalanced braces for {prefix}")
    end = i  # position after closing brace
    return text[start:end]


def build_file(name: str, body: str) -> str:
    includes = ['#include "pages/PageFactory.h"', '#include "PagesCommon.h"']
    if "AuthService::" in body:
        includes.append('#include "services/AuthService.h"')
    if "WorkOrderDetailDialog" in body:
        includes.append('#include "dialogs/WorkOrderDetailDialog.h"')
    return "\n".join(includes) + "\n\n" + body + "\n"


def main():
    origin_text = ORIGIN.read_text(encoding="utf-8")
    property_text = PROPERTY_PAGE.read_text(encoding="utf-8")

    # Extract and write each build function
    for name in NAMES:
        prefix = f"void PageFactory::buildProperty{name}("
        body = extract_function(origin_text, prefix)
        file_path = SRC / f"buildProperty{name}.cpp"
        file_path.write_text(build_file(name, body), encoding="utf-8")
        print(f"Created {file_path}")

    # Extract dispatcher from PropertyPage.cpp and place it into buildPropertySubPages.cpp
    dispatcher = extract_function(property_text, "BasePage *PageFactory::createPropertyPage(")
    dispatcher_file = '#include "pages/PageFactory.h"\n#include "PagesCommon.h"\n\n' + dispatcher + "\n"
    ORIGIN.write_text(dispatcher_file, encoding="utf-8")
    print(f"Replaced {ORIGIN} with dispatcher")

    # Reduce PropertyPage.cpp to includes only (do not delete the file)
    reduced = '#include "pages/PageFactory.h"\n#include "PagesCommon.h"\n'
    PROPERTY_PAGE.write_text(reduced, encoding="utf-8")
    print(f"Reduced {PROPERTY_PAGE} to includes only")


if __name__ == "__main__":
    main()
