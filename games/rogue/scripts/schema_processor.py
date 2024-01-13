#!/usr/bin/env python3
"""
Post-process JSON schema files to resolve references to other schema files
or internal references.
"""

import sys
import argparse
import json
from pathlib import Path
from typing import List

from xzen.schema import Schema, SchemaProcessor


def get_common_base_path(paths: List[Path]) -> Path:
    """
    Returns the common base path for a list of paths.
    """
    common_base_path = paths[0].parent

    for path in paths[1:]:
        while not path.is_relative_to(common_base_path):
            common_base_path = common_base_path.parent

    return common_base_path


def get_path_relative_to_base(path: Path, base: Path, new_base: Path) -> Path:
    """
    Returns a path relative to a new base path.
    """
    relative_path = path.relative_to(base)
    return new_base / relative_path


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--schema-files",
        metavar="SCHEMA_FILE",
        nargs="+",
        type=Path,
        help="JSON schema files to process",
    )
    parser.add_argument(
        "--output-dir",
        metavar="OUTPUT_DIR",
        help="Path to output directory to store processed JSON schemas",
    )
    args = parser.parse_args(argv[1:])

    # Load all schemas
    schemas: List[Schema] = []
    for schema_file in args.schema_files:
        with open(schema_file, "r") as f:
            schema_json = json.load(f)
        schemas.append(Schema(schema_json["$id"], schema_file, schema_json))

    # Resolve all references
    schema_processor = SchemaProcessor(schemas)
    schema_processor.resolve_all()

    # Write out processed schemas
    common_base_path = get_common_base_path(
        [Path(schema_file) for schema_file in args.schema_files]
    )
    for schema in schemas:
        schema_file = get_path_relative_to_base(
            schema.schema_file, common_base_path, Path(args.output_dir)
        )
        if not schema_file.parent.exists():
            schema_file.parent.mkdir(parents=True)
        with open(schema_file, "w") as f:
            json.dump(schema.data, f, indent=2)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
