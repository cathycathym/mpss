#!/usr/bin/env python3
"""
Reproduce the main score summaries reported in the MPSS paper.

Expected default input:
  results/results_60.csv

Required columns:
  problem_id, problem_name, set, Base, Prog, Repeat, Synth

`Repeat` should be blank for the initial40 rows.

Optional dependency for paired tests:
  pip install scipy
"""

from __future__ import annotations

import argparse
from pathlib import Path

import pandas as pd


METHODS = ["Base", "Prog", "Repeat", "Synth"]


def numeric(df: pd.DataFrame, col: str) -> pd.Series:
    return pd.to_numeric(df[col], errors="coerce")


def mean_score(series: pd.Series) -> float:
    return float(series.dropna().mean())


def full_solves(series: pd.Series) -> int:
    return int((series == 100).sum())


def compare(a: pd.Series, b: pd.Series) -> tuple[int, int, int]:
    mask = a.notna() & b.notna()
    a = a[mask]
    b = b[mask]
    return int((a > b).sum()), int((a == b).sum()), int((a < b).sum())


def wilcoxon_report(a: pd.Series, b: pd.Series, label: str) -> None:
    mask = a.notna() & b.notna()
    x = a[mask]
    y = b[mask]
    try:
        from scipy.stats import wilcoxon
        result = wilcoxon(x, y, alternative="two-sided", zero_method="wilcox")
        print(f"{label}: p={result.pvalue:.6g}")
    except Exception as exc:
        print(f"{label}: Wilcoxon unavailable ({exc})")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--results",
        default="results/results_60.csv",
        help="Canonical result CSV.",
    )
    args = parser.parse_args()

    path = Path(args.results)
    df = pd.read_csv(path)

    required = {"problem_id", "set", "Base", "Prog", "Repeat", "Synth"}
    missing = required - set(df.columns)
    if missing:
        raise ValueError(f"Missing required columns: {sorted(missing)}")

    if len(df) != 60:
        raise ValueError(f"Expected 60 rows, found {len(df)}.")

    for col in METHODS:
        df[col] = numeric(df, col)

    df["Best"] = df[["Base", "Prog"]].max(axis=1)

    print("=== All 60 problems ===")
    for col in ("Base", "Prog", "Best", "Synth"):
        s = df[col]
        print(
            f"{col:>5}: mean={mean_score(s):.3f}, "
            f"full={full_solves(s)}/{s.notna().sum()}"
        )

    better, tied, worse = compare(df["Synth"], df["Best"])
    print(
        f"Synth vs Best(Base, Prog): better={better}, tied={tied}, worse={worse}"
    )

    print("\nPaired Wilcoxon tests")
    wilcoxon_report(df["Synth"], df["Base"], "Synth vs Base")
    wilcoxon_report(df["Synth"], df["Prog"], "Synth vs Prog")
    wilcoxon_report(df["Synth"], df["Best"], "Synth vs Best")

    controlled = df[df["set"].astype(str).str.lower() == "controlled20"].copy()
    if len(controlled) != 20:
        raise ValueError(
            f"Expected 20 controlled20 rows, found {len(controlled)}."
        )

    print("\n=== Controlled 20 problems ===")
    for col in ("Base", "Prog", "Repeat", "Best", "Synth"):
        s = controlled[col]
        print(
            f"{col:>6}: mean={mean_score(s):.3f}, "
            f"full={full_solves(s)}/{s.notna().sum()}"
        )

    better, tied, worse = compare(controlled["Synth"], controlled["Repeat"])
    print(
        f"Synth vs Repeat: better={better}, tied={tied}, worse={worse}"
    )
    wilcoxon_report(
        controlled["Synth"],
        controlled["Repeat"],
        "Synth vs Repeat",
    )

    print("\n=== Paper-rounded values ===")
    for col in ("Base", "Prog", "Best", "Synth"):
        print(f"All60 {col}: {mean_score(df[col]):.1f}")
    for col in ("Base", "Prog", "Repeat", "Best", "Synth"):
        print(f"Controlled20 {col}: {mean_score(controlled[col]):.1f}")


if __name__ == "__main__":
    main()
