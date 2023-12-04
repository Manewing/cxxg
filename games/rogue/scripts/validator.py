#!/usr/bin/env python3
"""
Validates that JSON files conform to a schema.
"""

import sys
import json
import argparse
import jsonschema
import jsonschema.exceptions
import referencing
import referencing.exceptions

from pathlib import Path
from typing import List
from functools import partial


def retrieve_from_filesystem(url: str, local_path: Path, uri: str):
    if not uri.startswith(url):
        raise referencing.exceptions.NoSuchResource(ref=uri)
    path = local_path / Path(uri.removeprefix(url))
    contents = json.loads(path.read_text())
    return referencing.Resource.from_contents(contents)


def validate_json(json_file: str, schema_file: str, url: str):
    with open(json_file, "r") as f:
        json_data = json.load(f)

    with open(schema_file, "r") as f:
        schema_data = json.load(f)

    registry = referencing.Registry(
        retrieve=partial(
            retrieve_from_filesystem, url, Path(schema_file).parent
        )
    )
    jsonschema.validate(json_data, schema_data, registry=registry)
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
    parser.add_argument(
        "--url",
        type=str,
        dest="url",
        help="""
        JSON schema URL to use for resolving references
        """,
    )
    args = parser.parse_args(args[1:])

    for json_file in args.json_files:
        try:
            validate_json(json_file, args.schema_file, args.url)
        except jsonschema.exceptions.ValidationError as e:
            print(f"Validation error for: {json_file:s}")
            print(e)
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
