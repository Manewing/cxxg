#!/usr/bin/env python3

import math
import enum
from dataclasses import dataclass, field
from typing import List, Tuple


class ResourceKind(enum.Enum):
    WATER = "water"
    BERRIES = "berries"
    COCONUT = "coconut"
    WOOD = "wood"
    COTTON = "cotton"

    @classmethod
    def from_string(cls, string: str) -> 'ResourceKind':
        return cls[string.upper()]

    def __repr__(self) -> str:
        return self.value


@dataclass(frozen=True)
class Point2d:
    x: int
    y: int

    def euclidean_dist(self, other: 'Point2d') -> float:
        d_x = other.x - self.x
        d_y = other.y - self.y
        return math.sqrt(d_x * d_x + d_y * d_y)

    def manhattan_dist(self, other: 'Point2d') -> float:
        d_x = other.x - self.x
        d_y = other.y - self.y
        return float(abs(d_x) + abs(d_y))


@dataclass(frozen=True)
class Resource:
    # Kind of the resource
    kind: ResourceKind

    # Position of the resource
    pos: Point2d


@dataclass(frozen=True)
class Item:
    kind: str
    is_usable: bool


class WorldObjectKind(enum.Enum):
    CAMPFIRE = "campfire"

    @classmethod
    def from_string(cls, string: str) -> 'WorldObjectKind':
        return cls[string.upper()]

    def __repr__(self) -> str:
        return self.value


@dataclass(frozen=True)
class WorldObject:
    kind: WorldObjectKind
    pos: Point2d


@dataclass
class Inventory:
    items: List[Item] = field(default_factory=list)

    @property
    def num_items(self):
        return len(self.items)

    def add(self, item: Item):
        self.items.append(item)

    def pop(self, item_idx: int):
        return self.items.pop(item_idx)

    def remove(self, item: int):
        return self.items.remove(item)

    def has_items(self, item: Item, count: int) -> bool:
        return self.items.count(item) >= count

    def has_all_items(self, items: List[Tuple[Item, int]]) -> bool:
        return all(self.has_items(item, count) for item, count in items)

    def get_unique_items(self) -> List[Tuple[int, Item]]:
        unique_kinds = set()
        unique_items = list()
        for idx, item in enumerate(self.items):
            if not item.kind in unique_kinds:
                unique_kinds.add(item.kind)
                unique_items.append((idx, item))
        return unique_items


@dataclass
class State:
    health: int
    thirst: int
    hunger: int
    fatigue: int

    pos: Point2d

    resources: List[Resource] = field(default_factory=list)
    inventory: Inventory = field(default_factory=Inventory)
    world_objects: List[Item] = field(default_factory=list)

    def copy(self) -> 'State':
        return State(health=self.health,
                     thirst=self.thirst,
                     hunger=self.hunger,
                     fatigue=self.fatigue,
                     pos=self.pos,
                     resources=self.resources,
                     inventory=Inventory(list(self.inventory.items)),
                     world_objects=list(self.world_objects))
