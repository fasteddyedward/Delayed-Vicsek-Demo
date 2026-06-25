#!/usr/bin/env python3

"""
Summarize delayed Vicsek simulation outputs.

This script searches a run directory for simulation folders containing both
input.json and Order_parameters.txt. For each completed simulation, it computes
late-time averages over the final fraction of the recorded time series and writes
one compact summary table.

Example:
    python3 analysis/summarize_order_parameters.py \
        --run-dir example_slurm \
        --output analysis/polarization_summary.txt
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np


HEADER = (
    "N D_0 v_0 range delta_t dt J "
    "polarization pol_var\n"
)


def read_order_parameters(path: Path) -> np.ndarray:
    """Read Order_parameters.txt and return a 2D NumPy array."""
    data = np.loadtxt(path, skiprows=1)
    if data.ndim == 1:
        data = np.expand_dims(data, axis=1)

    # print(np.shape(data))
    return data


def summarize_single_run(input_json_path: Path, window_fraction=0.1) -> dict | None:
    """Summarize one simulation folder."""
    run_dir = input_json_path.parent
    order_parameter_path = run_dir / "Order_parameters.txt"

    if not order_parameter_path.is_file():
        print(f"Skipping {run_dir}: missing Order_parameters.txt")
        return None

    try:
        with input_json_path.open("r") as f:
            params = json.load(f)
    except Exception as exc:
        print(f"Skipping {run_dir}: could not read input.json: {exc}")
        return None

    try:
        data = read_order_parameters(order_parameter_path)
    except Exception as exc:
        print(f"Skipping {run_dir}: could not read Order_parameters.txt: {exc}")
        return None


    start = int((1.0 - window_fraction) * len(data))
    start = max(start, 0)
    late_time = data[start:]
    # print("start="+str(start))
    # print("len="+str(len(data)))
    # Column convention inherited from the simulation output:
    # 0: polarization
    polarization = late_time[:, 0]
    # print(polarization)
    return {
        "N": int(params.get("N", -1)),
        "D_0": float(params.get("D_0", np.nan)),
        "v_0": float(params.get("v_0", np.nan)),
        "range": float(params.get("range", np.nan)),
        "delta_t": float(params.get("delta_t", np.nan)),
        "dt": float(params.get("dt", np.nan)),
        "J": float(params.get("J", np.nan)),
        "polarization": float(np.mean(polarization)),
        "pol_var": float(np.var(polarization)),
    }


def write_summary(rows: list[dict], output_path: Path) -> None:
    """Write summary rows as a whitespace-separated table."""
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with output_path.open("w") as f:
        f.write(HEADER)

        for row in rows:
            f.write(
                f"{row['N']} "
                f"{row['D_0']:.6g} "
                f"{row['v_0']:.6g} "
                f"{row['range']:.6g} "
                f"{row['delta_t']:.6g} "
                f"{row['dt']:.6g} "
                f"{row['J']:.6g} "
                f"{row['polarization']:.6g} "
                f"{row['pol_var']:.6g}\n"
            )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Summarize delayed Vicsek simulation output folders."
    )

    parser.add_argument(
        "--run-dir",
        type=Path,
        required=True,
        help="Directory containing simulation output folders.",
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path("analysis/polarization_summary.txt"),
        help="Output summary table.",
    )

    parser.add_argument(
        "--window-fraction",
        type=float,
        default=0.1,
        help="Final fraction of the time series used for late-time averages.",
    )

    return parser.parse_args()


def main() -> None:
    args = parse_args()

    if not args.run_dir.is_dir():
        raise FileNotFoundError(f"Run directory not found: {args.run_dir}")

    input_files = sorted(args.run_dir.rglob("input.json"))

    if not input_files:
        raise FileNotFoundError(f"No input.json files found below {args.run_dir}")

    rows = []

    for input_json_path in input_files:
        row = summarize_single_run(input_json_path, args.window_fraction)
        if row is not None:
            rows.append(row)

    if not rows:
        raise RuntimeError("No valid simulation folders were summarized.")

    rows.sort(key=lambda r: (r["D_0"], r["delta_t"], r["J"], r["v_0"]))

    write_summary(rows, args.output)

    print(f"Processed {len(rows)} simulation folders.")
    print(f"Wrote summary to: {args.output}")


if __name__ == "__main__":
    main()