#!/usr/bin/env python3

import os
import sys
import json
import argparse
import dataclasses
from PIL import Image, ImageDraw, ImageFont
from typing import List, Dict, Union, ClassVar, Optional

from pyrogue.entity_db import EntityDb
from pyrogue.tiles_db import TilesDb


def get_entity_tiles(entity_db: EntityDb) -> Dict[str, dict]:
    entity_tiles = {}
    for idx, name in enumerate(entity_db.get_entity_names()):
        fully_defined_template = entity_db.get_fully_defined_entity(idx)
        assemblers = fully_defined_template.get("assemblers", {})
        if "tile" in assemblers:
            entity_tiles[name] = assemblers["tile"]
    return entity_tiles


@dataclasses.dataclass
class EntityTileInfo:
    key: str
    tile: Optional[dict] = None
    tile_type: ClassVar[str] = "entity"

    def to_dict(self) -> dict:
        return dict(key=self.key, tile_type=self.tile_type)

    @staticmethod
    def convert(entity_tiles: Dict[str, dict]) -> List["EntityTileInfo"]:
        return sorted(
            [
                EntityTileInfo(key=key, tile=tile)
                for key, tile in entity_tiles.items()
            ],
            key=lambda x: x.key,
        )


@dataclasses.dataclass
class ConfigTileInfo:
    key: str
    tile: dict
    tile_type: ClassVar[str] = "tile"

    def to_dict(self) -> dict:
        return dict(key=self.key, tile=self.tile, tile_type=self.tile_type)

    @staticmethod
    def convert(tiles: Dict[str, dict]) -> List["ConfigTileInfo"]:
        return sorted(
            [ConfigTileInfo(key=key, tile=tile) for key, tile in tiles.items()],
            key=lambda x: x.key,
        )


TileInfo = Union[EntityTileInfo, ConfigTileInfo]


class TilesIdMap:
    def __init__(self, tiles_id_map: List[TileInfo]):
        self.tiles_id_map = tiles_id_map

    @staticmethod
    def load(tiles_id_map_path: str) -> "TilesIdMap":
        with open(tiles_id_map_path, "r") as file:
            data = json.load(file)
        tiles_id_map = []
        for tile_info in data:
            tile_type = tile_info.get("tile_type")
            del tile_info["tile_type"]
            if tile_type == "entity":
                tile_info = EntityTileInfo(**tile_info)
            elif tile_type == "tile":
                tile_info = ConfigTileInfo(**tile_info)
            else:
                raise ValueError(f"Unknown tile type {tile_type}")
            tiles_id_map.append(tile_info)
        return TilesIdMap(tiles_id_map)

    def dump(self, stream) -> None:
        data = [tile_info.to_dict() for tile_info in self.tiles_id_map]
        json.dump(data, stream, indent=4, sort_keys=True)

    def write(self, tiles_id_map_path: str) -> None:
        with open(tiles_id_map_path, "w") as file:
            self.dump(file)

    def __len__(self):
        return len(self.tiles_id_map)

    def __iter__(self):
        return iter(self.tiles_id_map)

    def update(self, tiled_id_map: List[TileInfo]):
        entities = dict(
            (x.key, x) for x in self.tiles_id_map if x.tile_type == "entity"
        )
        tiles = dict(
            (x.key, x) for x in self.tiles_id_map if x.tile_type == "tile"
        )
        for tile_info in tiled_id_map:
            if tile_info.tile_type == "entity":
                if tile_info.key not in entities:
                    self.tiles_id_map.append(tile_info)
                    entities[tile_info.key] = tile_info
                elif tile_info.tile is not None:
                    entities[tile_info.key].tile = tile_info.tile
            else:
                if tile_info.key not in tiles:
                    self.tiles_id_map.append(tile_info)
                    tiles[tile_info.key] = tile_info
                elif tile_info.tile is not None:
                    tiles[tile_info.key].tile = tile_info.tile


MONO_FONT = os.path.join(os.path.dirname(__file__), "monofont.ttf")


class TileSetGenerator:
    def __init__(
        self,
        tiles_id_map: TilesIdMap,
        num_tiles_x: int,
    ):
        self.tiles_id_map = tiles_id_map
        self.num_tiles_x = num_tiles_x
        self.tile_width = 128
        self.tile_height = 128

        num_total_tiles = len(tiles_id_map)
        num_tiles_y = (num_total_tiles // num_tiles_x) + 1

        width = self.num_tiles_x * self.tile_width
        height = num_tiles_y * self.tile_height

        self.image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
        self.font = ImageFont.truetype(MONO_FONT, 120)
        self.small_font = ImageFont.truetype(MONO_FONT, 12)
        self.draw = ImageDraw.Draw(self.image)

        self.write_idx = 0

    def generate_tile(
        self, tile_cfg: Dict[str, str], key: Optional[str] = None
    ) -> Image:
        bg_color = tile_cfg.get("bg_color")
        if bg_color is None:
            tile = Image.new("RGBA", (self.tile_width, self.tile_height))
        else:
            tile = Image.new(
                "RGB", (self.tile_width, self.tile_height), bg_color
            )
        draw = ImageDraw.Draw(tile)

        char = tile_cfg["char"]
        color = tile_cfg["color"]

        # Calculate text position to center it
        _, _, text_width, text_height = self.font.getbbox(char)

        text_x = (self.tile_width - text_width) // 2
        text_y = (self.tile_height - text_height) // 2

        if key is not None:
            draw.text((0, 0), key, fill=(255, 0, 0), font=self.small_font)
        draw.text((text_x, text_y), char, fill=color, font=self.font)

        return tile

    def generate_tileset(self) -> int:
        for tile_info in self.tiles_id_map:
            x = (self.write_idx % self.num_tiles_x) * self.tile_width
            y = (self.write_idx // self.num_tiles_x) * self.tile_height
            key = None
            if tile_info.tile_type == "entity":
                key = tile_info.key
            tile = self.generate_tile(tile_info.tile, key=key)
            self.image.paste(tile, (x, y))
            self.write_idx += 1
        return self.write_idx

    def save(self, path: str) -> None:
        self.image.save(path)


def main(args: List[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Generate a tiled id map from a tileset image"
    )
    parser.add_argument(
        "--entity-db", type=str, required=True, help="The entity database"
    )
    parser.add_argument(
        "--tiles-db", type=str, required=True, help="The tiles database"
    )
    parser.add_argument(
        "--tile-set", type=str, required=True, help="The tileset image"
    )
    parser.add_argument(
        "--tile-width", default=16, type=int, help="The width of a tile"
    )
    parser.add_argument(
        "--tile-height", default=16, type=int, help="The height of a tile"
    )
    parser.add_argument(
        "--tiled-id-map",
        type=str,
        default=None,
        help="The tile id map to update",
    )
    args = parser.parse_args(args)

    entity_db = EntityDb.load(args.entity_db)
    tiles_db = TilesDb.load(args.tiles_db)

    entity_tiles = get_entity_tiles(entity_db)
    tiles = tiles_db.get_tiles()

    if args.tiled_id_map and os.path.exists(args.tiled_id_map):
        tiled_id_map = TilesIdMap.load(args.tiled_id_map)
    else:
        tiled_id_map = TilesIdMap([])
    tiled_id_map.update(EntityTileInfo.convert(entity_tiles))
    tiled_id_map.update(ConfigTileInfo.convert(tiles))

    if args.tiled_id_map:
        tiled_id_map.write(args.tiled_id_map)
    else:
        tiled_id_map.dump(sys.stdout)

    generator = TileSetGenerator(tiled_id_map, num_tiles_x=16)
    generator.generate_tileset()
    generator.save(args.tile_set)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
