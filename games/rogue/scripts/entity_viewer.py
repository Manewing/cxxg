#!/usr/bin/env python3

import os
import sys
import yaml
import argparse
from pathlib import Path
from functools import partial

import PySimpleGUI as sg
from PySimpleGUI.PySimpleGUI import Element
from typing import List, Optional, Callable

from xzen.ui import ListEditorBase
from xzen.ui import BaseInterface
from xzen.ui import BaseWindow

from xzen.ui_gen import JSONEditorGenerator
from xzen.ui_gen import BaseGeneratedEditor
from xzen.ui_gen import LinkedGeneratedEnumEditor
from xzen.ui_gen import JSONFileManagerInterface

from pyrogue.item_db import ItemDb
from pyrogue.entity_db import EntityDb
from pyrogue.tools import ToolError
from pyrogue.tools import RogueToolPaths
from pyrogue.tools import RogueToolWrapper
from pyrogue.tools import TempJsonFile
from pyrogue.tile_edit import TileEditor

SCHEMAS_PATH = Path(__file__).parent.parent / "data" / "schemas"
ENTITY_DB_SID = "https://rogue-todo.com/entity-db-schema.json"


class SelectEntityViewer(ListEditorBase):
    def __init__(
        self,
        parent: BaseInterface,
        entity_db: EntityDb,
        select_cb: Callable[[int], None],
        tool_paths: RogueToolPaths,
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
        self.tool_paths = tool_paths

    def get_element(self) -> Element:
        elem = super().get_element()
        toolbar = [
            sg.Text("todo"),
            sg.Button(
                "Show Cfg",
                tooltip="Show fully resolved entity configuration",
                key=self.k.show_cfg,
            ),
            sg.VSep(),
            sg.Button(
                "Arena",
                tooltip="View arena with entity",
                key=self.k.arena,
            ),
            sg.VSep(),
            sg.Button(
                "Player Arena",
                tooltip="View arena with player and entity",
                key=self.k.player_arena,
            ),
        ]
        self.register_event(self.k.show_cfg)
        self.register_event(self.k.arena)
        self.register_event(self.k.player_arena)
        layout = [toolbar, [sg.HSep(pad=20)], [elem]]
        return sg.Frame("Entities", layout)

    def handle_event(self, event: str, values: dict) -> bool:
        if super().handle_event(event, values):
            return True
        if event == self.k.show_cfg:
            idx = self.get_real_index()
            if idx >= len(self.values):
                return True
            entity = self.entity_db.get_fully_defined_entity(idx)
            entity_str = yaml.dump(entity, indent=2, sort_keys=True)
            sg.popup_scrolled(entity_str, title="Entity Configuration")

        if event == self.k.player_arena:
            idx = self.get_real_index()
            if idx >= len(self.values):
                return True

            wrapper = RogueToolWrapper(self.tool_paths)
            wrapper.build()

            arena_json_config = self.tool_paths.data.get_level("arena")
            arena_json = TempJsonFile(arena_json_config)

            test_game_config = self.tool_paths.data.test_game_config
            test_game_json = TempJsonFile(test_game_config)

            entity = self.values[idx]
            for str_idx in range(0, 10):
                arena_json.data["entities"][str(str_idx)] = "null"
            arena_json.data["entities"]["4"] = entity

            arena_path = arena_json.write()
            test_game_json.data["initial_level_config"] = str(
                arena_path.relative_to(test_game_config.parent)
            )

            try:
                test_game_path = test_game_json.write()
                wrapper.rogue([test_game_path], new_window=True)
            except ToolError as e:
                sg.popup_scrolled(e.output, title="Tool Error")
            return True

        if event == self.k.arena:
            idx = self.get_real_index()
            if idx >= len(self.values):
                return True

            # Create new terminal window for map viewer console application
            wrapper = RogueToolWrapper(self.tool_paths)
            wrapper.build()

            # Create a temporary file for the arena configuration
            arena_json_config = self.tool_paths.data.get_level("arena")
            arena_json = TempJsonFile(arena_json_config)

            entity = self.values[idx]
            for str_idx in range(1, 6):
                arena_json.data["entities"][str(str_idx)] = entity

            try:
                arena_path = arena_json.write()
                cmd = [self.tool_paths.data.test_game_config, arena_path]
                wrapper.map_viewer(cmd, new_window=True)
            except ToolError as e:
                sg.popup_scrolled(e.output, title="Tool Error")
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


# TODO:
# - YAML viewer for resolved entity templates
# - Use entity template resolver to resolve partial templates and compress them again
# - Integrate map viewer to watch entity in action
# - Add arena option to fight entity in arena
class EntityViewer(BaseWindow):
    def __init__(
        self, item_db: ItemDb, entity_db: EntityDb, tool_paths: RogueToolPaths
    ):
        super().__init__("Entity Viewer")
        self.item_db = item_db
        self.entity_db = entity_db
        self.entity_db.resolve_inheritance()
        self.tool_paths = tool_paths

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
                path,
                partial(
                    LinkedGeneratedEnumEditor, self.item_db.get_loot_table_names
                ),
            )

        self.current_entity = None
        self.entity_editor: Optional[BaseGeneratedEditor] = None

    def get_entity_layout(self) -> List[List[sg.Element]]:
        self.select_viewer = SelectEntityViewer(
            self, self.entity_db, self._on_select_entity, self.tool_paths
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
                    scrollable=(False, True),
                    justification="left",
                    key=self.k.entity_editor_col,
                    size=(900, None),
                    vertical_scroll_only=True,
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
            on_save=self._handle_save,
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

    def _handle_save(self, path: str) -> None:
        copy = self.entity_db.copy()
        copy.compress_inheritance()
        copy.save(path)

    def _handle_load(self, path: str) -> None:
        self.entity_db.load_from(path)
        self.entity_db.resolve_inheritance()
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
    parser.add_argument(
        "--bin_dir", required=True, help="Path to Rogue bin directory"
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

    tool_paths = RogueToolPaths(pargs.bin_dir)

    item_db = ItemDb.load(item_db_path)
    entity_db = EntityDb.load(entity_db_path)

    viewer = EntityViewer(item_db, entity_db, tool_paths)
    viewer.setup()
    viewer.run()

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
