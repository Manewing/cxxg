#!/usr/bin/env python3

import json
import dacite
from dataclasses import dataclass

from obj_dbs import ItemDb
from state import ResourceKind, State, WorldObjectKind, Item


@dataclass
class Config:
    description: str
    goal_heuristic: str
    initial_state: State


def load_config(config: str) -> Config:
    with open(config, "r") as f:
        config_dict = json.load(f)
    type_hooks = {
        ResourceKind: ResourceKind.from_string,
        WorldObjectKind: WorldObjectKind.from_string,
        Item: ItemDb.get
    }
    dacitecfg = dacite.Config(type_hooks=type_hooks)
    return dacite.from_dict(data_class=Config,
                            data=config_dict,
                            config=dacitecfg)
