#!/usr/bin/env python3
"""
Cleans a configuration (JSON/YAML) file by sorting keys and indenting.
"""

import os
import sys
import json
import yaml
import argparse


def clean(target: str) -> None:
    if os.path.isdir(target):
        print(f"Skipping directory: {target}", file=sys.stderr)
        return 0
    if target.endswith(".yaml"):
        with open(target, "r") as input_file:
            data = yaml.load(input_file, Loader=yaml.SafeLoader)

        with open(target, "w") as output_file:
            yaml.dump(data, output_file, indent=2, sort_keys=True)

        return 0

    if not target.endswith(".json"):
        print(
            f"Target file must end with .json or .yaml: {target}",
            file=sys.stderr,
        )
        return 1

    with open(target, "r") as input_file:
        data = json.load(input_file)

    with open(target, "w") as output_file:
        json.dump(data, output_file, indent=2, sort_keys=True)

    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "targets", metavar="TARGETS", nargs="+", help="Target file to clean"
    )
    args = parser.parse_args()

    ret = 0
    for target in args.targets:
        ret += clean(target)
    return ret


if __name__ == "__main__":
    sys.exit(main())
