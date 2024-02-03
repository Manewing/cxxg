#!/usr/bin/env python3

import json
from typing import List, Dict


class TilesDb:
    def __init__(self, tiles_db: Dict[str, dict]):
        self.tiles_db = tiles_db

    def get_tiles(self) -> Dict[str, dict]:
        return self.tiles_db

    @staticmethod
    def load(tiles_db_path: str) -> "TilesDb":
        with open(tiles_db_path, "r") as file:
            tiles_db = json.load(file)
        return TilesDb(tiles_db)

    def write(self, tiles_db_path: str) -> None:
        with open(tiles_db_path, "w") as file:
            json.dump(self.tiles_db, file, indent=4, sort_keys=True)