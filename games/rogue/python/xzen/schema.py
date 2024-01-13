#!/usr/bin/env python3
"""
Classes for (post-)processing JSON schemas.
"""

import dataclasses
from pathlib import Path
from typing import List, Any, Dict, Set


@dataclasses.dataclass
class Schema:
    """
    Represents a JSON schema loaded from file.
    """

    schema_id: str
    schema_file: Path
    data: Dict[str, Any]

    schema_ref_keys: Set[str] = dataclasses.field(default_factory=set)


@dataclasses.dataclass
class SchemaRef:
    """
    A reference to value in another schema
    """

    # The schema that the reference is in
    schema: Schema

    # The value that is referenced
    value: Any

    # The key of the value in the parent
    key: Any

    # The parent of the value
    parent: Any


@dataclasses.dataclass
class SchemaProcessor:
    """
    Post-process JSON schema files to resolve references to other schema files
    or internal references.
    """

    schemas: List[Schema]
    schemas_by_id: Dict[str, Schema] = dataclasses.field(default_factory=dict)
    resolved_schema_ids: Set[str] = dataclasses.field(default_factory=set)

    def __post_init__(self) -> None:
        """
        Builds a dictionary of schemas by schema ID.
        """
        for schema in self.schemas:
            self.schemas_by_id[schema.schema_id] = schema

    def resolve_all(self, remove_keys: bool = True) -> None:
        """
        Resolves references for all schemas.
        """
        for schema in self.schemas:
            self.resolve(schema)
        if remove_keys:
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

    @staticmethod
    def normalize_path(path: str) -> str:
        """
        Normalizes internal or external reference path, removes leading,
        trailing "/"
        """
        if path.startswith("#"):
            return "#" + SchemaProcessor.normalize_ref_path(path[1:])
        schema_id, ref_path = path.split("#", 1)
        return schema_id + "#" + SchemaProcessor.normalize_ref_path(ref_path)

    @staticmethod
    def normalize_ref_path(ref_path: str) -> str:
        """
        Normalizes a reference path, removes leading, trailing "/"
        """
        return ref_path.lstrip("/").rstrip("/")

    def resolve_ref_path(self, schema: Schema, ref_path: str) -> SchemaRef:
        """
        Resolves a reference path.
        """
        ref_path = self.normalize_ref_path(ref_path)
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
