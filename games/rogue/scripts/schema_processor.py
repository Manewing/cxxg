#!/usr/bin/env python3
"""
Post-process JSON schema files to resolve references to other schema files
or internal references.
"""

import sys
import argparse
import json
import dataclasses
from pathlib import Path
from typing import List, Any, Dict, Set, Tuple


@dataclasses.dataclass
class Schema:
    schema_id: str
    schema_file: Path
    data: Dict[str, Any]

    schema_ref_keys: Set[str] = dataclasses.field(default_factory=set)


@dataclasses.dataclass
class SchemaRef:
    schema: Schema
    value: Any
    key: Any
    parent: Any


@dataclasses.dataclass
class SchemaProcessor:
    schemas: List[Schema]
    schemas_by_id: Dict[str, Schema] = dataclasses.field(default_factory=dict)
    resolved_schema_ids: Set[str] = dataclasses.field(default_factory=set)

    def __post_init__(self) -> None:
        """
        Builds a dictionary of schemas by schema ID.
        """
        for schema in self.schemas:
            self.schemas_by_id[schema.schema_id] = schema

    def resolve_all(self) -> None:
        """
        Resolves references for all schemas.
        """
        for schema in self.schemas:
            self.resolve(schema)
        for schema in self.schemas:
            self.remove_ref_keys(schema)

    def resolve(self, schema: Schema) -> None:
        """
        Resolves references for the schema.
        """
        for key, value in schema.data.items():
            self.process_value(schema, schema.data, key, value)
        self.resolved_schema_ids.add(schema.schema_id)

    def remove_ref_keys(self, schema: Schema) -> None:
        """
        Removes all reference keys from the schema.
        """
        for key in schema.schema_ref_keys:
            try:
                del schema.data[key]
            except KeyError as e:
                raise ValueError(
                    f"{schema.schema_file}: Invalid reference key: {key}"
                ) from e

    def process_value(
        self, schema: Schema, parent: Any, key: Any, value: Any
    ) -> None:
        if isinstance(value, dict):
            self.process_dict(schema, parent, key, value)
        elif isinstance(value, list):
            self.process_list(schema, value)

    def process_dict(
        self, schema: Schema, parent: Any, key: Any, value: Dict[str, Any]
    ) -> None:
        if "$ref" in value:
            ref = self.resolve_ref(schema, value["$ref"])
            self.process_value(
                schema=ref.schema,
                parent=ref.parent,
                key=ref.key,
                value=ref.value,
            )
            parent[key] = ref.parent[ref.key]
            return

        for sub_key, sub_value in value.items():
            self.process_value(schema, value, sub_key, sub_value)

    def process_list(self, schema: Schema, value: List[Any]) -> None:
        for sub_key, sub_value in enumerate(value):
            self.process_value(schema, value, sub_key, sub_value)

    def resolve_ref(self, schema: Schema, ref: str) -> SchemaRef:
        if ref.startswith("#"):
            return self.resolve_internal_ref(schema, ref)
        return self.resolve_external_ref(schema, ref)

    def resolve_internal_ref(self, schema: Schema, ref: str) -> SchemaRef:
        """
        Resolves an internal reference.
        """
        schema_id, ref_path = ref.split("#", 1)
        if schema_id != "":
            raise ValueError(
                f"{schema.schema_file}: Invalid schema ID in reference: {ref}"
            )
        return self.resolve_ref_path(schema, ref_path)

    def resolve_external_ref(self, schema: Schema, ref: str) -> SchemaRef:
        """
        Resolves an external reference.
        """
        schema_id, ref_path = ref.split("#", 1)
        if schema_id not in self.schemas_by_id:
            raise ValueError(
                f"{schema.schema_file}: Invalid schema ID in reference: {ref}"
            )
        ref_schema = self.schemas_by_id[schema_id]
        if schema_id not in self.resolved_schema_ids:
            self.resolve(ref_schema)
        return self.resolve_ref_path(ref_schema, ref_path)

    def resolve_ref_path(self, schema: Schema, ref_path: str) -> SchemaRef:
        """
        Resolves a reference path.
        """
        ref_path = ref_path.lstrip("/")
        ref_path_parts = ref_path.split("/")
        ref_value = schema.data
        ref_parent = schema.data
        schema.schema_ref_keys.add(ref_path_parts[0])
        try:
            for ref_path_part in ref_path_parts:
                ref_parent = ref_value
                ref_value = ref_value[ref_path_part]
        except KeyError as e:
            raise ValueError(
                f"{schema.schema_file}: Invalid reference path: {ref_path}"
            ) from e
        return SchemaRef(schema, ref_value, ref_path_parts[-1], ref_parent)


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
