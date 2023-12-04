#!/usr/bin/env python3
"""
Post-processes the entity database JSON to resolve configuration parameters
inherited from other entity templates.
"""

import sys
import argparse
import json
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

        while from_template is not None:
            base_entity_template = self.entity_templates_by_name[from_template]
            self.resolve_from(entity_template, base_entity_template)
            from_template = base_entity_template.get("from_template")

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


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--entity-db", help="Path to entity database JSON")
    parser.add_argument("--output", help="Path to output JSON")
    args = parser.parse_args(argv)

    with open(args.entity_db, "r") as f:
        entity_db = json.load(f)

    entity_templates = entity_db["entity_templates"]

    resolver = InheritanceResolver(entity_templates)
    resolver.resolve_all()

    with open(args.output, "w") as f:
        json.dump(entity_db, f, indent=4, sort_keys=True)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
