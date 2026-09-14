#!/usr/bin/env python3
"""
Clean release script for the final MPSS experiment.

This script matches the repository terminology and layout used in the paper:

  generations/
    initial40/<problem_id>/{Base,Prog,Synth}.cpp
    controlled20/<problem_id>/{Base,Prog,Repeat,Synth}.cpp

It loads the exact prompt templates from prompts/, selects the frozen problem
IDs from the official SubtaskEval dataset, and makes independent model calls.

Important:
- This is a clean reference implementation of the final protocol.
- The released generated programs and QOJ results are the canonical artifacts
  used in the paper.
- No online-judge result or correctness feedback is provided during generation.

Requirements:
  pip install openai

Environment:
  export OPENAI_API_KEY=...

"""

from __future__ import annotations

import argparse
import json
import os
import re
import urllib.request
from pathlib import Path

from openai import OpenAI

from extract_code import extract_cpp


DATASET_URL = "https://zenodo.org/records/17370525/files/dataset.jsonl?download=1"

INITIAL40 = [
    7157, 9161, 9184, 9163, 10049, 10050, 13167, 13168, 11401, 11403,
    13171, 8707, 8708, 8709, 8710, 8711, 8712, 8266, 8269, 8271,
    62, 185, 192, 360, 676, 789, 795, 948, 2498, 3123,
    3141, 3502, 5181, 7160, 8255, 8640, 9164, 10399, 10400, 11445,
]

CONTROLLED20 = [
    5514, 6332, 10881, 10882, 10884, 10886, 8256, 8257, 8258, 8259,
    8641, 8643, 8645, 8646, 8648, 8649, 8651, 6333, 9165, 9166,
]


def qoj_id(link: str) -> int | None:
    m = re.search(r"/problem/(\d+)(?:$|[/?#])", str(link))
    return int(m.group(1)) if m else None


def ensure_dataset(path: Path) -> None:
    if path.exists() and path.stat().st_size > 1000:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    req = urllib.request.Request(
        DATASET_URL,
        headers={"User-Agent": "mpss-llm-reproduction/1.0"},
    )
    with urllib.request.urlopen(req, timeout=120) as response:
        path.write_bytes(response.read())


def load_dataset(path: Path) -> dict[int, dict]:
    rows: dict[int, dict] = {}
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            row = json.loads(line)
            pid = qoj_id(row.get("link", ""))
            if pid is not None:
                rows[pid] = row
    return rows


def problem_block(row: dict) -> str:
    parts = []
    for key in (
        "name",
        "problem_statement_md",
        "io_and_constraints",
        "samples_with_explanations",
        "subtasks",
    ):
        value = row.get(key)
        if value:
            parts.append(f"## {key.replace('_', ' ').upper()}\n{value}")
    return "\n\n".join(parts)


def load_prompt(repo_root: Path, name: str) -> str:
    path = repo_root / "prompts" / f"{name.lower()}_prompt.txt"
    if not path.exists():
        raise FileNotFoundError(f"Missing prompt file: {path}")
    return path.read_text(encoding="utf-8")


def call_model(
    client: OpenAI,
    model: str,
    reasoning_effort: str,
    prompt: str,
    max_output_tokens: int,
) -> str:
    response = client.responses.create(
        model=model,
        reasoning={"effort": reasoning_effort},
        max_output_tokens=max_output_tokens,
        input=prompt,
    )
    return response.output_text


def generate_condition(
    *,
    client: OpenAI,
    model: str,
    reasoning_effort: str,
    max_output_tokens: int,
    template: str,
    problem: str,
    output_path: Path,
    base_code: str | None = None,
    prog_code: str | None = None,
) -> None:
    if output_path.exists():
        print(f"[skip] {output_path}")
        return

    values = {"problem_block": problem}
    if base_code is not None:
        values["base_code"] = base_code
    if prog_code is not None:
        values["prog_code"] = prog_code

    prompt = template.format(**values)
    raw = call_model(
        client=client,
        model=model,
        reasoning_effort=reasoning_effort,
        prompt=prompt,
        max_output_tokens=max_output_tokens,
    )
    code = extract_cpp(raw)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(code, encoding="utf-8")
    print(f"[write] {output_path} ({len(code.encode('utf-8'))} bytes)")


def run_problem(
    *,
    repo_root: Path,
    client: OpenAI,
    row: dict,
    set_name: str,
    model: str,
    reasoning_effort: str,
    max_output_tokens: int,
) -> None:
    pid = qoj_id(row["link"])
    if pid is None:
        raise RuntimeError("Problem row has no QOJ ID.")

    dest = repo_root / "generations" / set_name / str(pid)
    block = problem_block(row)

    base_template = load_prompt(repo_root, "base")
    prog_template = load_prompt(repo_root, "prog")
    repeat_template = load_prompt(repo_root, "repeat")
    synth_template = load_prompt(repo_root, "synth")

    base_path = dest / "Base.cpp"
    prog_path = dest / "Prog.cpp"

    generate_condition(
        client=client,
        model=model,
        reasoning_effort=reasoning_effort,
        max_output_tokens=max_output_tokens,
        template=base_template,
        problem=block,
        output_path=base_path,
    )
    generate_condition(
        client=client,
        model=model,
        reasoning_effort=reasoning_effort,
        max_output_tokens=max_output_tokens,
        template=prog_template,
        problem=block,
        output_path=prog_path,
    )

    if set_name == "controlled20":
        repeat_path = dest / "Repeat.cpp"
        generate_condition(
            client=client,
            model=model,
            reasoning_effort=reasoning_effort,
            max_output_tokens=max_output_tokens,
            template=repeat_template,
            problem=block,
            output_path=repeat_path,
        )

    # Synth uses only the problem, Base, and Prog.
    base_code = base_path.read_text(encoding="utf-8")
    prog_code = prog_path.read_text(encoding="utf-8")
    synth_path = dest / "Synth.cpp"
    generate_condition(
        client=client,
        model=model,
        reasoning_effort=reasoning_effort,
        max_output_tokens=max_output_tokens,
        template=synth_template,
        problem=block,
        base_code=base_code,
        prog_code=prog_code,
        output_path=synth_path,
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--set",
        choices=["initial40", "controlled20", "all"],
        default="all",
        help="Which frozen problem set to generate.",
    )
    parser.add_argument(
        "--dataset",
        default="dataset.jsonl",
        help="Path to the official SubtaskEval dataset.jsonl.",
    )
    parser.add_argument(
        "--model",
        default=os.environ.get("OPENAI_MODEL", "gpt-5.6-sol"),
    )
    parser.add_argument(
        "--reasoning-effort",
        choices=["low", "medium", "high"],
        default="medium",
    )
    parser.add_argument("--max-output-tokens", type=int, default=32768)
    args = parser.parse_args()

    if not os.environ.get("OPENAI_API_KEY"):
        raise SystemExit("OPENAI_API_KEY is not set.")

    repo_root = Path(__file__).resolve().parent.parent
    dataset_path = Path(args.dataset)
    if not dataset_path.is_absolute():
        dataset_path = repo_root / dataset_path

    ensure_dataset(dataset_path)
    dataset = load_dataset(dataset_path)

    selected_sets: list[tuple[str, list[int]]] = []
    if args.set in ("initial40", "all"):
        selected_sets.append(("initial40", INITIAL40))
    if args.set in ("controlled20", "all"):
        selected_sets.append(("controlled20", CONTROLLED20))

    missing = [
        pid
        for _, ids in selected_sets
        for pid in ids
        if pid not in dataset
    ]
    if missing:
        raise RuntimeError(f"Missing QOJ IDs in dataset: {sorted(set(missing))}")

    client = OpenAI()

    for set_name, ids in selected_sets:
        print(f"\n=== {set_name} ===")
        for i, pid in enumerate(ids, 1):
            print(f"[{i:02d}/{len(ids)}] QOJ {pid}: {dataset[pid].get('name', '')}")
            run_problem(
                repo_root=repo_root,
                client=client,
                row=dataset[pid],
                set_name=set_name,
                model=args.model,
                reasoning_effort=args.reasoning_effort,
                max_output_tokens=args.max_output_tokens,
            )


if __name__ == "__main__":
    main()
