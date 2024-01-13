#!/usr/bin/env python3

import sys
import json
import argparse
from pathlib import Path
from typing import List, Any

import PySimpleGUI as sg

from xzen.schema import Schema
from xzen.schema import SchemaProcessor

from xzen.ui_gen import GeneratedJsonEditor


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
        "--path",
        metavar="PATH",
        required=True,
        help="Path to object to generate UI for",
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

    ref = schema_processor.resolve_external_ref(None, args.path)

    sg.theme("DarkAmber")
    sg.set_options(font=("Courier New", 16))

    gen_window = GeneratedJsonEditor(ref.key, ref.value)

    print(ref.key)
    print(gen_window.get_value())
    gen_window.setup()
    gen_window.run()
    print(gen_window.get_value())

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
