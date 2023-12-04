#!/usr/bin/env python3

from typing import List, Tuple
from dataclasses import dataclass

from state import Inventory, Item

class ItemDb:
    _ITEMS = {
        "berry": Item("berry", True),
        "wood": Item("wood", False),
        "cotton": Item("cotton", False),
        "coconut": Item("coconut", True),
        "campfire": Item("campfire", True),
        "bandage": Item("bandage", True),
    }

    @classmethod
    def get(cls, item_kind: str) -> Item:
        item = cls._ITEMS[item_kind.lower()]
        assert item.kind == item_kind
        return item


@dataclass
class CraftingInfo:
    result_item: Item
    required_items: List[Tuple[Item, int]]


class CraftingDb:
    _ALL_CRAFTABLES = [
        CraftingInfo(result_item=ItemDb.get("campfire"),
                     required_items=[(ItemDb.get("wood"), 5)]),
        CraftingInfo(result_item=ItemDb.get("bandage"),
                     required_items=[(ItemDb.get("wood"), 1),
                                     (ItemDb.get("cotton"), 3)])
    ]

    @classmethod
    def get_craftables(cls, inventory: Inventory) -> List[CraftingInfo]:
        craftables = []
        for cinfo in cls._ALL_CRAFTABLES:
            if inventory.has_all_items(cinfo.required_items):
                craftables.append(cinfo)
        return craftables