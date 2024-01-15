#!/usr/bin/env python3
"""
Post-processes the entity database JSON to resolve configuration parameters
inherited from other entity templates.
"""

import sys
import argparse
import json
from typing import List

from pyrogue.entity_db import EntityDb
from pyrogue.entity_db import InheritanceResolver


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--entity-db", help="Path to entity database JSON")
    parser.add_argument("--output", help="Path to output JSON")
    args = parser.parse_args(argv)

    entity_db = EntityDb.load(args.entity_db)
    resolver = InheritanceResolver(entity_db.get_templates())
    resolver.resolve_all()

    entity_db.save(args.output)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
