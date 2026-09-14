#!/usr/bin/env python3
"""
Extract exactly one C++ code block from a saved model response.

The generation protocol asks for exactly one ```cpp ... ``` block.
If zero or multiple C++ blocks are present, extraction fails and returns
an empty string so the generation failure remains visible.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


def extract_cpp(text: str) -> str:
    blocks = re.findall(r"```cpp\s*(.*?)```", text, flags=re.IGNORECASE | re.DOTALL)
    if len(blocks) != 1:
        return ""
    return blocks[0].strip()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", help="Text file containing the model response.")
    parser.add_argument("output", help="Destination .cpp file.")
    args = parser.parse_args()

    src = Path(args.input)
    dst = Path(args.output)

    code = extract_cpp(src.read_text(encoding="utf-8"))
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_text(code, encoding="utf-8")

    if code:
        print(f"Extracted {len(code.encode('utf-8'))} bytes to {dst}")
    else:
        print("No unique C++ code block found; wrote an empty file.")


if __name__ == "__main__":
    main()
