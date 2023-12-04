#!/usr/bin/env python3

import math
from abc import ABC, abstractmethod
from typing import List
from copy import deepcopy
from dataclasses import dataclass

from state import Resource, ResourceKind, State, Item, WorldObject, WorldObjectKind
from obj_dbs import CraftingDb, ItemDb, CraftingInfo


class ActionBase(ABC):
    @abstractmethod
    def apply_to(self, state: State):
        """
        """

    @abstractmethod
    def compute_cost(self, state: State) -> float:
        """
        Cost for applying the action to the current state, must not be zero
        """

    def get_state_from(self, state: State) -> State:
        new_state = state.copy()
        self.apply_to(new_state)
        return new_state


@dataclass
class GatherResource(ActionBase):
    resource: Resource

    def apply_to(self, state: State) -> State:
        if self.resource.kind == ResourceKind.WATER:
            state.thirst += 1000
        elif self.resource.kind == ResourceKind.BERRIES:
            state.inventory.add(ItemDb.get("berry"))
        elif self.resource.kind == ResourceKind.WOOD:
            state.inventory.add(ItemDb.get("wood"))
        elif self.resource.kind == ResourceKind.COTTON:
            state.inventory.add(ItemDb.get("cotton"))
        elif self.resource.kind == ResourceKind.COCONUT:
            state.inventory.add(ItemDb.get("coconut"))

        state.pos = self.resource.pos

    def compute_cost(self, state: State) -> float:
        distance = int(self.resource.pos.manhattan_dist(state.pos))
        if distance != 0:
            return math.log2(distance) + 5
        return 5


@dataclass
class UseItem(ActionBase):
    item_idx: int
    item: Item

    def apply_to(self, state: State):
        if self.item.kind == "berry":
            state.hunger += 1000
        elif self.item.kind == "coconut":
            state.thirst += 200
            state.hunger += 400
        elif self.item.kind == "bandage":
            state.health += 400
        elif self.item.kind == "campfire":
            world_obj = WorldObject(WorldObjectKind.CAMPFIRE, state.pos)
            state.world_objects.append(world_obj)
        else:
            assert False, f"invalid item kind {self.item.kind}"
        state.inventory.pop(self.item_idx)

    def compute_cost(self, state: State) -> float:
        return 5.0


@dataclass
class CraftItem(ActionBase):
    cinfo: CraftingInfo

    def apply_to(self, state: State):
        for item, count in self.cinfo.required_items:
            for _ in range(count):
                state.inventory.remove(item)
        state.inventory.add(self.cinfo.result_item)

    def compute_cost(self, state: State) -> float:
        return 5.0


@dataclass
class ObjectInteract(ActionBase):
    world_obj: WorldObject

    def apply_to(self, state: State):
        if self.world_obj.kind == WorldObjectKind.CAMPFIRE:
            # sleep at campfire
            state.fatigue += 1000
        else:
            assert False, f"invalid object kind {self.world_obj.kind}"

    def compute_cost(self, state: State) -> float:
        distance = int(self.world_obj.pos.manhattan_dist(state.pos))
        if distance != 0:
            return math.log2(distance) + 5
        return 5.0


def get_available_actions(state: State) -> List[ActionBase]:
    actions: List[ActionBase] = []
    for resource in state.resources:
        actions.append(GatherResource(resource))
    for idx, item in state.inventory.get_unique_items():
        if item.is_usable:
            actions.append(UseItem(idx, item))
    for cinfo in CraftingDb.get_craftables(state.inventory):
        actions.append(CraftItem(cinfo))
    for world_obj in state.world_objects:
        actions.append(ObjectInteract(world_obj))
    return actions
