#!/usr/bin/env python3

import sys
import json
import argparse
from pathlib import Path
from typing import List, Any, Dict, Set

import PySimpleGUI as sg

from schema_processor import Schema
from schema_processor import SchemaProcessor


def convert_snake_case_to_camel_case(s: str) -> str:
    """
    Converts a string from snake_case to camelCase.
    """
    return "".join(word.capitalize() for word in s.split("_"))


def get_title(key: str, obj: Dict[str, Any]) -> str:
    """
    Returns the title for the given object.
    """
    title = obj.get("title")
    if title:
        return title
    return convert_snake_case_to_camel_case(key)


def generate_ui_for_string_enum(
    key: str, obj: Dict[str, Any]
) -> List[sg.Element]:
    """
    Generates a UI for the given string enum.
    """
    title = get_title(key, obj)
    description = obj.get("description")
    enum_values = obj["enum"]
    return [
        sg.Text(title, tooltip=description),
        sg.Combo(
            enum_values,
            default_value=enum_values[0],
            enable_events=True,
            key=key,
            tooltip=description,
        ),
    ]

def generate_input(key: str, obj: Dict[str, Any]) -> List[sg.Element]:
    """
    Generates a UI for the given input.
    """
    title = get_title(key, obj)
    description = obj.get("description")
    return [
        sg.Text(title, tooltip=description),
        sg.InputText(
            default_text=obj.get("default"),
            enable_events=True,
            key=key,
            tooltip=description,
        ),
    ]

def generate_color_input(key: str, obj: Dict[str, Any]) -> List[sg.Element]:
    """
    Generates a UI for the given color input.
    """
    title = get_title(key, obj)
    description = obj.get("description")
    return [
        sg.Text(title, tooltip=description),
        sg.InputText(
            default_text=obj.get("default"),
            enable_events=True,
            key=key,
            tooltip=description,
        ),
        sg.ColorChooserButton("Choose color", target=key),
    ]

def generate_ui_for_string(key: str, obj: Dict[str, Any]) -> List[sg.Element]:
    if "enum" in obj:
        return generate_ui_for_string_enum(key, obj)
    if "pattern" in obj and obj["pattern"] == "#[0-9a-fA-F]{6}":
        return generate_color_input(key, obj)

    return generate_input(key, obj)

def generate_ui_for_object(key: str, obj: Dict[str, Any]) -> List[sg.Element]:
    properties = obj["properties"]
    layout = []
    for sub_key, sub_value in properties.items():
        layout += [generate_ui(sub_key, sub_value)]
    return [
        sg.Frame(
            get_title(key, obj),
            [[sg.Column(layout)]],
            tooltip=obj.get("description"),
        )
    ]

def generate_ui(key: Any, obj: Any) -> List[sg.Element]:
    """
    Generates a UI for the given object.
    """
    if not isinstance(key, str):
        raise ValueError(f"Expected str, got: {key}")
    if not isinstance(obj, dict):
        raise ValueError(f"Expected dict, got: {obj}")
    obj_type = obj.get("type")
    if not obj_type:
        raise ValueError(f"Expected type in object, got: {obj_type}")
    handlers = {
        "string": generate_ui_for_string,
        "object": generate_ui_for_object,
        "integer": generate_input,
    }
    if obj_type not in handlers:
        raise ValueError(f"Unsupported type: {obj_type}")
    return handlers[obj_type](key, obj)


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

    row = generate_ui(ref.key, ref.value)
    print(row)
    layout = [row]

    window = sg.Window("Schema UI", layout)
    while True:
        event, values = window.read()
        if event == sg.WIN_CLOSED:
            break
        print(event, values)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
