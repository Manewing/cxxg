#!/usr/bin/env python3

import json
from typing import List, Optional


class ItemDb:
    def __init__(self, item_db: dict, item_db_path: Optional[str] = None):
        self.item_db = item_db
        self.item_db_path = item_db_path

    @staticmethod
    def load(item_db_path: str) -> "ItemDb":
        with open(item_db_path, "r") as f:
            item_db = json.load(f)
        return ItemDb(item_db, item_db_path)

    def load_from(self, item_db_path: str) -> None:
        with open(item_db_path, "r") as f:
            self.item_db = json.load(f)
        self.item_db_path = item_db_path

    def save(self, item_db_path: str) -> None:
        with open(item_db_path, "w") as f:
            json.dump(self.item_db, f, indent=2, sort_keys=True)

    def get_loot_table_names(self) -> List[str]:
        return sorted(self.item_db["loot_tables"].keys())

    def get_item_names(self) -> List[str]:
        return list(x["name"] for x in self.item_db["item_prototypes"])

    def get_specialization_names(self) -> List[str]:
        return list(x["name"] for x in self.item_db["item_specializations"])

    def get_item_effect_names(self) -> List[str]:
        return sorted(x for x in self.item_db["item_effects"])

    def get_item_effect_names_and_defaults(self) -> List[str]:
        defaults = [
            "null",
            "sweeping_strike_effect",
            "smite_effect",
            "remove_poison_effect",
            "remove_poison_debuff",
            "learn_crafting_recipe",
        ]
        return sorted(defaults + self.get_item_effect_names())

    def get_loot_table(self, loot_table_name: str) -> dict:
        return self.item_db["loot_tables"][loot_table_name]

    def set_loot_table(self, loot_table_name: str, loot_table: dict) -> None:
        self.item_db["loot_tables"][loot_table_name] = loot_table

    def set_item(self, item_idx: int, item: dict) -> None:
        self.item_db["item_prototypes"][item_idx] = item
