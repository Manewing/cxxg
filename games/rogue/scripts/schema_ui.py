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
from xzen.ui_gen import JSONEditorGenerator


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
    generator = JSONEditorGenerator.load(args.schema_files)
    editor = generator.create_editor_interface_from_path(args.path)
    gen_window = GeneratedJsonEditor(editor)

    print(gen_window.get_value())
    gen_window.setup()
    gen_window.run()
    print(gen_window.get_value())

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
