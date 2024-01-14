#!/usr/bin/env python3

import os
import sys
import json
import argparse
from pathlib import Path
from functools import partial

import PySimpleGUI as sg
from PySimpleGUI.PySimpleGUI import Element
from typing import List, Optional, Callable, Dict, Any

from xzen.ui import ListEditorBase
from xzen.ui import BaseInterface
from xzen.ui import BaseWindow

from xzen.ui_gen import JSONEditorGenerator
from xzen.ui_gen import BaseGeneratedEditor
from xzen.ui_gen import GeneratedObjectEditor
from xzen.ui_gen import LinkedGeneratedEnumEditor
from xzen.ui_gen import JSONFileManagerInterface

from pyrogue.item_db import ItemDb


SCHEMAS_PATH = Path(__file__).parent.parent / "data" / "schemas"
ENTITY_DB_SID = "https://rogue-todo.com/entity-db-schema.json"


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

    def get_entity_names(self) -> List[str]:
        return list(x["name"] for x in self.entity_db["entity_templates"])

    def set_entity(self, entity_idx: int, entity: dict) -> None:
        self.entity_db["entity_templates"][entity_idx] = entity

    def get_entity(self, entity_idx: int) -> dict:
        return self.entity_db["entity_templates"][entity_idx]

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


class TileEditor(GeneratedObjectEditor):
    def __init__(
        self,
        generator: JSONEditorGenerator,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface] = None,
        prefix: str = "",
    ):
        super().__init__(generator, key, obj, parent, prefix)
        self.description = "Tile" if not self.description else self.description

    def get_header(self) -> List[List[Element]]:
        header = super().get_header()
        color = self.obj.get("color", "#FFFFFF")
        bg_color = self.obj.get("bg_color", "#000000")
        header[0].append(
            sg.Frame(
                "Char",
                [
                    [
                        sg.Text(
                            self.obj.get("char", "@"),
                            text_color=color,
                            background_color=bg_color,
                            font=("Courier New", 30),
                            key=self.k.char_disp
                        )
                    ]
                ],
            )
        )
        return header

    def update_ui(self) -> None:
        super().update_ui()
        obj = self.get_value()
        self.w.char_disp.update(
            obj.get("char", "@"),
            text_color=obj.get("color", "#FFFFFF"),
            background_color=obj.get("bg_color", "#000000"),
        )


class SelectEntityViewer(ListEditorBase):
    def __init__(
        self,
        parent: BaseInterface,
        entity_db: EntityDb,
        select_cb: Callable[[int], None],
    ):
        super().__init__(
            select_cb,
            parent,
            modifiable=True,
            orderable=True,
            description="List of loot tables, click to select",
            values=entity_db.get_entity_names(),
            on_add_item=self.on_add_item,
            on_rm_item=self.on_rm_item,
            on_move_up=self.on_move_up,
            on_move_down=self.on_move_down,
            size=(40, 35),
        )
        self.entity_db = entity_db

    def get_element(self) -> Element:
        elem = super().get_element()
        toolbar = [
            sg.VSep(),
        ]
        layout = [toolbar, [sg.HSep(pad=20)], [elem]]
        return sg.Frame("Loot Tables", layout)

    def handle_event(self, event: str, values: dict) -> bool:
        if super().handle_event(event, values):
            return True
        return False

    def on_add_item(self) -> Optional[str]:
        entity_tmpl_name = sg.popup_get_text(
            "Enter entity template name",
            title="Add entity template",
            default_text=self.w.search.get(),
        )
        if not entity_tmpl_name:
            return
        if entity_tmpl_name in self.entity_db.get_entity_names():
            sg.popup(f"Entity template already exists: {entity_tmpl_name}")
            return
        self.entity_db.create_new_entity(entity_tmpl_name)
        return entity_tmpl_name

    def on_rm_item(self, idx: int) -> None:
        del self.entity_db.entity_db["entity_templates"][idx]

    def on_move_up(self, idx: int) -> None:
        values = self.entity_db.entity_db["entity_templates"]
        values[idx - 1], values[idx] = values[idx], values[idx - 1]

    def on_move_down(self, idx: int) -> None:
        values = self.entity_db.entity_db["entity_templates"]
        values[idx + 1], values[idx] = values[idx], values[idx + 1]


class EntityViewer(BaseWindow):
    def __init__(self, item_db: ItemDb, entity_db: EntityDb):
        super().__init__("Loot Viewer")
        self.item_db = item_db
        self.entity_db = entity_db

        schema_files = SCHEMAS_PATH.glob("*_schema.json")

        self.generator = JSONEditorGenerator.load(schema_files)

        # Custom handling for tiles
        assembler_path = ENTITY_DB_SID + "#defs/entity_template/assemblers"
        tile_paths = [
            f"{assembler_path}/tile/object",
            f"{assembler_path}/door/object/closed_tile",
            f"{assembler_path}/door/object/open_tile",
            f"{assembler_path}/loot_interact/object/default_tile",
            f"{assembler_path}/loot_interact/object/looted_tile",
        ]
        for path in tile_paths:
            self.generator.register_override(path, TileEditor)

        # Register link enum editors to override keys referencing other data
        # in the same or in other documents (e.g. item names, loot table names)
        loot_table_paths = [
            f"{assembler_path}/inventory/object/loot_table",
        ]
        for path in loot_table_paths:
            self.generator.register_override(
                path, partial(LinkedGeneratedEnumEditor, self.item_db.get_loot_table_names)
            )

        self.current_entity = None
        self.entity_editor: Optional[BaseGeneratedEditor] = None

    def get_entity_layout(self) -> List[List[sg.Element]]:
        self.select_viewer = SelectEntityViewer(
            self,
            self.entity_db,
            self._on_select_entity,
        )
        self.entity_editor = self.generator.create_editor_interface_from_path(
            ENTITY_DB_SID + "#defs/entity_template", self
        )
        self.entity_editor.register_on_change_handler(self._on_entity_edited)
        return [
            [
                self.select_viewer.get_element(),
                sg.VSep(),
                sg.Column(
                    [self.entity_editor.get_row()],
                    expand_x=True,
                    expand_y=True,
                    scrollable=True,
                    key=self.k.entity_editor_col,
                    size=(800, 350),
                ),
            ],
        ]

    def get_file_tab_layout(self) -> List[List[sg.Element]]:
        self.item_db_file_manager = JSONFileManagerInterface(
            title="Item Database",
            on_load=self._handle_item_db_load,
            path=self.item_db.item_db_path,
            parent=self,
            prefix=f"{self.prefix}_item_db",
        )
        self.entity_db_file_manager = JSONFileManagerInterface(
            title="Entity Database",
            on_save=self.entity_db.save,
            on_load=self._handle_load,
            path=self.entity_db.entity_db_path,
            parent=self,
            prefix=f"{self.prefix}_entity_db",
        )
        layout = [
            [self.item_db_file_manager.get_element()],
            [sg.HSep()],
            [self.entity_db_file_manager.get_element()],
        ]
        return [[sg.Frame("File", layout, expand_x=True, expand_y=True)]]

    def _handle_item_db_load(self, path: str) -> None:
        self.item_db.load_from(path)
        self.entity_editor.set_value(
            self.entity_db.get_entity(self.current_entity),
            trigger_handlers=False,
            update_ui=True,
        )

    def _handle_load(self, path: str) -> None:
        self.entity_db.load_from(path)
        self.select_viewer.set_values(
            self.entity_db.get_entity_names(),
            trigger_handlers=True,
            update_ui=True,
        )

    def get_layout(self) -> List[List[Element]]:
        entity_layout = self.get_entity_layout()
        entity_tab = sg.Tab("Entity", entity_layout)

        file_tab_layout = self.get_file_tab_layout()
        file_tab = sg.Tab("File", file_tab_layout)

        self.register_refresh_handler(self._on_ui_refresh)

        return [[sg.TabGroup([[file_tab, entity_tab]])]]

    def _on_entity_edited(
        self, value: dict, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if self.current_entity is None:
            return
        self.entity_db.set_entity(self.current_entity, value)

        # Values in list are based on names which may change
        self.select_viewer.set_values(
            self.entity_db.get_entity_names(),
            trigger_handlers=False,
            update_ui=True,
        )

    def _on_select_entity(self, idx: int) -> None:
        self.current_entity = idx
        self.entity_editor.set_value(
            self.entity_db.get_entity(self.current_entity),
            trigger_handlers=False,
            update_ui=True,
        )

    def _on_ui_refresh(self) -> None:
        self.w.entity_editor_col.contents_changed()


def is_windows() -> bool:
    return os.name == "nt"


def adapt_exc_path(path: str) -> str:
    if is_windows():
        return path + ".exe"
    return path


def main(args: List[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Modify and view entity data from Rogue"
    )
    parser.add_argument(
        "--item_db", required=True, help="Path to item database"
    )
    parser.add_argument(
        "--entity_db", required=True, help="Path to entity database"
    )
    pargs = parser.parse_args(args)

    item_db_path = os.path.abspath(pargs.item_db)
    if not os.path.exists(item_db_path):
        print(f"Item database does not exist: {item_db_path}")
        return 1

    entity_db_path = os.path.abspath(pargs.entity_db)
    if not os.path.exists(entity_db_path):
        print(f"Entity database does not exist: {entity_db_path}")
        return 1

    item_db = ItemDb.load(item_db_path)
    entity_db = EntityDb.load(entity_db_path)

    viewer = EntityViewer(item_db, entity_db)
    viewer.setup()
    viewer.run()

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
