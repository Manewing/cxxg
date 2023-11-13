#!/usr/bin/env python3
"""
Validates that JSON files conform to a schema.
"""

import sys
import json
import argparse
import jsonschema
import jsonschema.exceptions

from typing import List


def validate_json(json_file: str, schema_file: str):
    with open(json_file, "r") as f:
        json_data = json.load(f)

    with open(schema_file, "r") as f:
        schema_data = json.load(f)

    jsonschema.validate(json_data, schema_data)
    print(f"Validated {json_file:s} against {schema_file:s}")


def main(args: List[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--json",
        type=str,
        nargs="+",
        dest="json_files",
        help="""
        JSON files to validate.
        """,
    )
    parser.add_argument(
        "--schema",
        type=str,
        dest="schema_file",
        help="""
        JSON schema file to validate against.
        """,
    )
    args = parser.parse_args(args[1:])

    for json_file in args.json_files:
        try:
            validate_json(json_file, args.schema_file)
        except jsonschema.exceptions.ValidationError as e:
            print(f"Validation error for: {json_file:s}")
            print(e)
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
