#!/usr/bin/env python3
import argparse
import csv
import re
import sys
from pathlib import Path


MODEL_RE = re.compile(r"^Model name:\s*(.+?)\s*$")
EVENT_TIME_RE = re.compile(r"^Event time:\s*([0-9]+(?:\.[0-9]+)?)\s+seconds\s*$")


def parse_log(path):
    cpu = None
    event_time = None

    with path.open("r", encoding="utf-8", errors="replace") as log_file:
        for line in log_file:
            if cpu is None:
                model_match = MODEL_RE.match(line)
                if model_match:
                    cpu = model_match.group(1)
                    continue

            time_match = EVENT_TIME_RE.match(line)
            if time_match:
                event_time = time_match.group(1)

    return cpu, event_time


def format_indices(indices):
    return ", ".join(str(index) for index in indices)


def main():
    parser = argparse.ArgumentParser(
        description="Extract CPU model and event time from Geant4 out_*.out logs."
    )
    parser.add_argument(
        "--log-dir",
        default="log",
        type=Path,
        help="Directory containing out_<index>.out files. Default: log",
    )
    parser.add_argument(
        "-o",
        "--output",
        default="cpu_time.csv",
        type=Path,
        help="Output CSV file. Default: cpu_time.csv",
    )
    parser.add_argument("--start", default=0, type=int, help="First job index. Default: 1")
    parser.add_argument("--end", default=2999, type=int, help="Last job index. Default: 3000")
    args = parser.parse_args()

    missing = []
    incomplete = []

    with args.output.open("w", newline="", encoding="utf-8") as output_file:
        writer = csv.writer(output_file)

        for job_index in range(args.start, args.end + 1):
            log_path = args.log_dir / f"out_{job_index}.out"

            if not log_path.exists():
                missing.append(job_index)
                writer.writerow([job_index, "", ""])
                continue

            cpu, event_time = parse_log(log_path)
            if cpu is None or event_time is None:
                incomplete.append(job_index)

            writer.writerow([job_index, cpu or "", event_time or ""])

    if missing:
        print(
            f"Missing log files: {len(missing)} ({format_indices(missing)})",
            file=sys.stderr,
        )
    if incomplete:
        print(
            f"Incomplete log files: {len(incomplete)} ({format_indices(incomplete)})",
            file=sys.stderr,
        )


if __name__ == "__main__":
    main()
