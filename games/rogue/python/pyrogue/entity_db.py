#!/usr/bin/env python3
"""
Entity database and related classes.
"""

import json
import copy
import dataclasses
from typing import List, Any, Dict, Optional, TypedDict


class EntityTemplate(TypedDict):
    """
    Represents (parts) of an entity template.
    """

    name: str
    from_template: Optional[str]
    assemblers: Dict[str, Any]


@dataclasses.dataclass
class InheritanceResolver:
    """
    Resolves inherited parameters for an entity template.
    """

    entity_templates: List[EntityTemplate]
    entity_templates_by_name: Dict[str, EntityTemplate] = dataclasses.field(
        default_factory=dict
    )

    def __post_init__(self) -> None:
        """
        Builds a dictionary of entity templates by name.
        """
        for entity_template in self.entity_templates:
            self.entity_templates_by_name[
                entity_template["name"]
            ] = entity_template

    def resolve_all(self) -> None:
        """
        Resolves inherited parameters for all entity templates.
        """
        for entity_template in self.entity_templates:
            self.resolve(entity_template)

    def resolve(self, entity_template: EntityTemplate) -> None:
        """
        Resolves inherited parameters for the entity template.
        """
        from_template = entity_template.get("from_template")

        visited_templates = set()
        while from_template is not None:
            if from_template in visited_templates:
                raise ValueError(
                    f"Circular inheritance detected for entity template: {from_template}"
                )
            visited_templates.add(from_template)
            base_entity_template = self.entity_templates_by_name[from_template]
            self.resolve_from(entity_template, base_entity_template)
            from_template = base_entity_template.get("from_template")

    def build(self, entity_template: EntityTemplate) -> EntityTemplate:
        """
        Builds inherited parameters for the entity template.
        """
        from_template = entity_template.get("from_template")
        if from_template is None:
            return entity_template

        # Create list from parent to the current entity template
        entity_template_list = [entity_template]
        while from_template is not None:
            from_entity_tmpl = self.entity_templates_by_name[from_template]
            entity_template_list.append(from_entity_tmpl)
            from_template = from_entity_tmpl.get("from_template")

        # Reverse the list so that the current entity template is first
        entity_template_list.reverse()
        build_entity_template = dict()
        for entity_tmpl in entity_template_list:
            for key, value in entity_tmpl.items():
                if key == "assemblers":
                    build_entity_template.setdefault(key, dict())
                    build_entity_template[key].update(value)
                else:
                    build_entity_template[key] = value

        return build_entity_template

    @staticmethod
    def resolve_from(
        entity_template: EntityTemplate,
        base_entity_template: EntityTemplate,
    ) -> None:
        assemblers = entity_template["assemblers"]
        base_assemblers = base_entity_template["assemblers"]

        for base_asm_name, base_asm_data in base_assemblers.items():
            asm_data = assemblers.get(base_asm_name)
            if not isinstance(base_asm_data, dict) or not isinstance(
                asm_data, dict
            ):
                continue
            for param_name, param_value in base_asm_data.items():
                if param_name in asm_data:
                    continue
                asm_data[param_name] = param_value


@dataclasses.dataclass
class InheritanceCompressor:
    """
    Inverse of InheritanceResolver. Removes inherited parameters from entity
    if they are not changed in the derived entity
    """

    entity_templates: List[EntityTemplate]
    entity_templates_by_name: Dict[str, EntityTemplate] = dataclasses.field(
        default_factory=dict
    )

    def __post_init__(self) -> None:
        """
        Builds a dictionary of entity templates by name.
        """
        for entity_template in self.entity_templates:
            self.entity_templates_by_name[
                entity_template["name"]
            ] = entity_template

    def compress_all(self) -> None:
        """
        Compresses inherited parameters for all entity templates.
        """
        for entity_template in self.entity_templates:
            self.compress(entity_template)

    def compress(self, entity_template: EntityTemplate) -> None:
        """
        Compresses inherited parameters for the entity template.
        """
        from_template = entity_template.get("from_template")

        while from_template is not None:
            base_entity_template = self.entity_templates_by_name[from_template]
            self.compress_from(entity_template, base_entity_template)
            from_template = base_entity_template.get("from_template")

    @staticmethod
    def compress_from(
        entity_template: EntityTemplate,
        base_entity_template: EntityTemplate,
    ) -> None:
        assemblers = entity_template["assemblers"]
        base_assemblers = base_entity_template["assemblers"]

        for base_asm_name, base_asm_data in base_assemblers.items():
            asm_data = assemblers.get(base_asm_name)
            if not isinstance(base_asm_data, dict) or not isinstance(
                asm_data, dict
            ):
                continue
            for param_name, param_value in base_asm_data.items():
                if param_name in asm_data:
                    if asm_data[param_name] == param_value:
                        del asm_data[param_name]


class EntityDb:
    def __init__(self, entity_db: dict, entity_db_path: Optional[str] = None):
        self.entity_db = entity_db
        self.entity_db_path = entity_db_path

    @staticmethod
    def load(entity_db_path: str) -> "EntityDb":
        with open(entity_db_path, "r") as f:
            entity_db = json.load(f)
        return EntityDb(entity_db, entity_db_path)

    def load_from(self, entity_db_path: str) -> None:
        with open(entity_db_path, "r") as f:
            self.entity_db = json.load(f)
        self.entity_db_path = entity_db_path

    def save(self, entity_db_path: str) -> None:
        with open(entity_db_path, "w") as f:
            json.dump(self.entity_db, f, indent=2, sort_keys=True)

    def copy(self) -> "EntityDb":
        return EntityDb(copy.deepcopy(self.entity_db), self.entity_db_path)

    def get_templates(self) -> List[dict]:
        return self.entity_db["entity_templates"]

    def get_entity_by_name(self, name: str) -> dict:
        for entity in self.entity_db["entity_templates"]:
            if entity["name"] == name:
                return entity
        raise ValueError(f"Entity not found: {name}")

    def get_entity_names(self) -> List[str]:
        return list(x["name"] for x in self.entity_db["entity_templates"])

    def set_entity(self, entity_idx: int, entity: dict) -> None:
        self.entity_db["entity_templates"][entity_idx] = entity

    def get_entity(self, entity_idx: int) -> dict:
        return self.entity_db["entity_templates"][entity_idx]

    def get_fully_defined_entity(self, entity_idx: int) -> dict:
        resolver = InheritanceResolver(self.get_templates())
        return resolver.build(self.get_entity(entity_idx))

    def add_entity(self, entity: dict) -> None:
        self.entity_db["entity_templates"].append(entity)

    def create_new_entity(self, name: str) -> dict:
        entity = {
            "name": name,
            "assemblers": {},
        }
        if name in self.get_entity_names():
            raise ValueError(f"Entity already exists: {name}")
        self.add_entity(entity)
        return entity

    def resolve_inheritance(self) -> None:
        """
        Resolves inheritance for all entity templates.
        """
        resolver = InheritanceResolver(self.get_templates())
        resolver.resolve_all()

    def compress_inheritance(self) -> None:
        """
        Compresses inheritance for all entity templates.
        """
        compressor = InheritanceCompressor(self.get_templates())
        compressor.compress_all()
