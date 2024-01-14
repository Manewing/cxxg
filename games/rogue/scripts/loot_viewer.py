#!/usr/bin/env python3

import os
import sys
import json
import argparse
import subprocess
from pathlib import Path
from functools import partial

import PySimpleGUI as sg
from PySimpleGUI.PySimpleGUI import Element
from matplotlib import pyplot as plt
from typing import List, Optional, Callable, Dict, Any

from xzen.ui import ListEditorBase
from xzen.ui import BaseInterface
from xzen.ui import BaseWindow

from xzen.ui_gen import JSONEditorGenerator
from xzen.ui_gen import BaseGeneratedEditor
from xzen.ui_gen import GeneratedArrayEditor
from xzen.ui_gen import LinkedGeneratedEnumEditor
from xzen.ui_gen import JSONFileManagerInterface


SCHEMAS_PATH = Path(__file__).parent.parent / "data" / "schemas"
LOOT_TB_SID = "https://rogue-todo.com/loot_tb_schema.json"
ITEM_DB_SID = "https://rogue-todo.com/item-db-schema.json"


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
        return list(x["name"] for x in self.item_db["item_effects"])

    def get_loot_table(self, loot_table_name: str) -> dict:
        return self.item_db["loot_tables"][loot_table_name]

    def set_loot_table(self, loot_table_name: str, loot_table: dict) -> None:
        self.item_db["loot_tables"][loot_table_name] = loot_table

    def set_item(self, item_idx: int, item: dict) -> None:
        self.item_db["item_prototypes"][item_idx] = item


class LootInfoWrapper:
    def __init__(
        self, item_db_path: str, schema_path: str, loot_info_excel_path: str
    ):
        self.item_db_path = item_db_path
        self.schema_path = schema_path
        self.loot_info_excel_path = loot_info_excel_path

    def get_loot_info(self, loot_table_name: str, rolls: int = 10000) -> dict:
        cmd = [
            self.loot_info_excel_path,
            self.item_db_path,
            self.schema_path,
            "--loot-table",
            loot_table_name,
            str(rolls),
        ]
        output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        output = output.decode("utf-8")
        output = output.replace("'", '"')
        try:
            return json.loads(output)
        except json.decoder.JSONDecodeError:
            print(f"Failed to decode JSON:", file=sys.stderr)
            print(output, file=sys.stderr)
            raise


class SelectLootTableViewer(ListEditorBase):
    def __init__(
        self,
        parent: BaseInterface,
        item_db: ItemDb,
        loot_info_wrapper: LootInfoWrapper,
        select_cb: Callable[[int], None],
    ):
        super().__init__(
            select_cb,
            parent,
            modifiable=True,
            orderable=False,
            description="List of loot tables, click to select",
            values=sorted(item_db.get_loot_table_names()),
            on_add_item=self.on_add_item,
            on_rm_item=self.on_rm_item,
            size=(40, 35),
        )
        self.item_db = item_db
        self.loot_info_wrapper = loot_info_wrapper

    def get_element(self) -> Element:
        elem = super().get_element()
        toolbar = [
            sg.Button(
                "Show Statistics",
                key=self.k.show_stats,
                expand_x=True,
            ),
        ]
        self.register_event(self.k.show_stats)

        layout = [toolbar, [sg.HSep(pad=20)], [elem]]
        return sg.Frame("Loot Tables", layout)

    def get_selected_loot_table(self):
        return self.values[self.get_real_index()]

    def handle_event(self, event: str, values: dict) -> bool:
        if super().handle_event(event, values):
            return True
        if event == self.k.show_stats:
            self.show_loot_table_stat_plot(self.get_selected_loot_table())
            return True
        return False

    def on_add_item(self) -> Optional[str]:
        loot_table_name = sg.popup_get_text(
            "Enter loot table name",
            title="Add Loot Table",
            default_text=self.w.search.get(),
        )
        if not loot_table_name:
            return
        if loot_table_name in self.item_db.get_loot_table_names():
            sg.popup(f"Loot table already exists: {loot_table_name}")
            return
        loot_table = {
            "rolls": 0,
            "slots": [],
        }
        self.item_db.item_db["loot_tables"][loot_table_name] = loot_table
        return loot_table_name

    def on_rm_item(self, idx: int) -> None:
        del self.item_db.item_db["loot_tables"][self.values[idx]]

    def show_loot_table_stat_plot(self, loot_table_name: str) -> None:
        self.item_db.save(self.loot_info_wrapper.item_db_path)
        try:
            loot_info = self.loot_info_wrapper.get_loot_info(loot_table_name)
        except subprocess.CalledProcessError as e:
            output = e.output.decode("utf-8")
            if output:
                sg.popup(f"Failed to get loot info:\n{output}")
            else:
                sg.popup(
                    f"Failed to get loot info:\n{' '.join(e.cmd)}\n"
                    f"Produced no output and returned: {e.returncode}"
                )
            return

        # Create bar char of loot table item rewards percentages
        item_names = [x["name"] for x in loot_info["item_rewards"]]
        item_cp_drops = [x["cp_drop"] for x in loot_info["item_rewards"]]
        item_percentages = [x["pp_drop"] for x in loot_info["item_rewards"]]

        fig = plt.figure(figsize=(14, 8))
        fig.set_tight_layout(True)

        ax = fig.add_subplot(111)
        rolls = loot_info["rolls"]
        total = loot_info["total"]
        title = f"Loot Table: {loot_table_name} ({rolls} rolls) ({total} items x{total / rolls:.2f})"
        ax.set_title(title)
        ax.set_xlabel("Item")
        x_pos = [i for i, _ in enumerate(item_names)]
        ax.set_xticks(x_pos)
        ax.set_xticklabels(item_names, rotation=90, fontsize=8)

        ax_cp_drops = ax
        ax_percentages = ax_cp_drops.twinx()

        lbl_cc_drops = "Count per drop"
        ax_cp_drops.set_ylabel(lbl_cc_drops)
        lbl_percentages = "Percentage per drop"
        ax_percentages.set_ylabel(lbl_percentages)
        x_pos = [i + 0.05 for i, _ in enumerate(item_names)]
        l1 = ax_cp_drops.bar(
            x_pos, item_cp_drops, color="tab:blue", width=0.2, align="edge"
        )

        x_pos = [i - 0.25 for i, _ in enumerate(item_names)]
        l2 = ax_percentages.bar(
            x_pos, item_percentages, color="tab:red", width=0.2, align="edge"
        )

        fig.legend((l1, l2), (lbl_cc_drops, lbl_percentages), loc="upper left")

        plt.show(block=False)


class SelectItemTableViewer(ListEditorBase):
    def __init__(
        self,
        parent: BaseInterface,
        item_db: ItemDb,
        select_cb: Callable[[int], None],
    ):
        super().__init__(
            select_cb,
            parent,
            modifiable=True,
            orderable=False,
            description="List of items, click to select",
            values=item_db.get_item_names(),
            on_add_item=self.on_add_item,
            on_rm_item=self.on_rm_item,
            size=(40, 35),
        )
        self.item_db = item_db

    def get_element(self) -> Element:
        elem = super().get_element()
        return sg.Frame("Items", [[elem]])

    def on_add_item(self) -> Optional[str]:
        item_name = sg.popup_get_text(
            "Enter item name",
            title="Add Item",
            default_text=self.w.search.get(),
        )
        if not item_name:
            return
        if item_name in self.item_db.get_item_names():
            sg.popup(f"Item already exists: {item_name}")
            return
        self.item_db.item_db["item_prototypes"].append({"name": item_name})
        return item_name

    def on_rm_item(self, idx: int) -> None:
        del self.item_db.item_db["item_prototypes"][idx]["name"]


class LootSlotsEditor(GeneratedArrayEditor):
    def __init__(
        self,
        generator: JSONEditorGenerator,
        key: str,
        obj: dict,
        parent: BaseInterface,
        prefix: str,
    ):
        super().__init__(generator, key, obj, parent, prefix)
        self.list_ui.size = (40, 10)

    def get_item_text(self, idx: int) -> str:
        slot = self.values[idx]
        prefix = f"({slot['weight']:3d}): "
        if slot["type"] == "item":
            min_count = slot["min_count"]
            max_count = slot["max_count"]
            return f"{prefix} Item: {slot['name']} ({min_count}-{max_count})"
        if slot["type"] == "table":
            return f"{prefix} Table: {slot['ref']}"
        if slot["type"] == "null":
            return f"{prefix} Null"
        raise ValueError(f"Invalid slot type: {slot['type']}")


class LootViewer(BaseWindow):
    def __init__(self, item_db: ItemDb, loot_info_wrapper: LootInfoWrapper):
        super().__init__("Loot Viewer")
        self.item_db = item_db
        self.loot_info_wrapper = loot_info_wrapper

        schema_files = SCHEMAS_PATH.glob("*_schema.json")

        self.generator = JSONEditorGenerator.load(schema_files)
        self.generator.register_override(
            LOOT_TB_SID + "#defs/loot_table/slots", LootSlotsEditor
        )
        self.generator.register_override(
            LOOT_TB_SID + "#defs/loot_table/slots/item/table/ref",
            partial(
                LinkedGeneratedEnumEditor, self.item_db.get_loot_table_names
            ),
        )
        self.generator.register_override(
            LOOT_TB_SID + "#defs/loot_table/slots/item/item/name",
            partial(LinkedGeneratedEnumEditor, self.item_db.get_item_names),
        )
        self.generator.register_override(
            ITEM_DB_SID + "#properties/item_prototypes/items/enhancements",
            partial(
                LinkedGeneratedEnumEditor,
                self.item_db.get_loot_table_names,
            ),
        )

        self.current_table = None
        self.loot_editor: Optional[BaseGeneratedEditor] = None

        self.current_item = None
        self.item_editor: Optional[BaseGeneratedEditor] = None

    def get_loot_table_layout(self) -> List[List[sg.Element]]:
        self.select_loot_table = SelectLootTableViewer(
            self,
            self.item_db,
            self.loot_info_wrapper,
            self._on_select_loot_table,
        )
        self.loot_editor = self.generator.create_editor_interface_from_path(
            LOOT_TB_SID + "#defs/loot_table", self
        )
        self.loot_editor.register_on_change_handler(self._on_loot_table_edited)
        return [
            [
                self.select_loot_table.get_element(),
                sg.VSep(),
                sg.Column(
                    [self.loot_editor.get_row()],
                    expand_x=True,
                    expand_y=True,
                    scrollable=True,
                    key=self.k.loot_editor_col,
                ),
            ],
        ]

    def get_item_tb_layout(self) -> List[List[sg.Element]]:
        self.select_item_table = SelectItemTableViewer(
            self,
            self.item_db,
            self._on_select_item,
        )
        self.item_editor = self.generator.create_editor_interface_from_path(
            ITEM_DB_SID + "#properties/item_prototypes/items",
            self,
        )
        self.item_editor.register_on_change_handler(self._on_item_edited)
        self.register_refresh_handler(self._on_ui_refresh)
        return [
            [
                self.select_item_table.get_element(),
                sg.VSep(),
                sg.Column(
                    [self.item_editor.get_row()],
                    expand_x=True,
                    expand_y=True,
                    scrollable=True,
                    key=self.k.item_editor_col,
                ),
            ],
        ]

    def get_file_tab_layout(self) -> List[List[sg.Element]]:
        self.item_db_file_manager = JSONFileManagerInterface(
            title="Item Database",
            on_save=self.item_db.save,
            on_load=self._handle_load,
            path=self.item_db.item_db_path,
            parent=self,
            prefix=self.prefix,
        )
        layout = [[self.item_db_file_manager.get_element()]]
        return [[sg.Frame("File", layout, expand_x=True, expand_y=True)]]

    def _handle_load(self, path: str) -> None:
        self.item_db.load_from(path)
        self.select_loot_table.set_values(
            self.item_db.get_loot_table_names(),
            trigger_handlers=True,
            update_ui=True,
        )
        self.select_item_table.set_values(
            self.item_db.get_item_names(),
            trigger_handlers=True,
            update_ui=True,
        )

    def get_layout(self) -> List[List[Element]]:
        loot_tb_layout = self.get_loot_table_layout()
        loot_tb_tab = sg.Tab("Loot Tables", loot_tb_layout)

        item_tb_layout = self.get_item_tb_layout()
        item_tb_tab = sg.Tab("Items", item_tb_layout)

        file_tab_layout = self.get_file_tab_layout()
        file_tab = sg.Tab("File", file_tab_layout)

        return [[sg.TabGroup([[file_tab, loot_tb_tab, item_tb_tab]])]]

    def _on_loot_table_edited(
        self, value: dict, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if self.current_table is None:
            return
        self.item_db.set_loot_table(self.current_table, value)

    def _on_select_loot_table(self, idx: int) -> None:
        self.current_table = self.select_loot_table.values[idx]
        self.loot_editor.set_value(
            self.item_db.get_loot_table(self.current_table),
            trigger_handlers=False,
            update_ui=True,
        )

    def _on_item_edited(
        self, value: dict, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if self.current_item is None:
            return
        self.loot_editor.update_ui()
        self.item_db.set_item(self.current_item, value)
        self.select_item_table.set_values(
            self.item_db.get_item_names(),
            trigger_handlers=False,
            update_ui=True,
        )

    def _on_select_item(self, idx: int) -> None:
        self.current_item = idx
        self.item_editor.set_value(
            self.item_db.item_db["item_prototypes"][self.current_item],
            trigger_handlers=False,
            update_ui=True,
        )

    def _on_ui_refresh(self) -> None:
        self.w.loot_editor_col.contents_changed()
        self.w.item_editor_col.contents_changed()


def is_windows() -> bool:
    return os.name == "nt"


def adapt_exc_path(path: str) -> str:
    if is_windows():
        return path + ".exe"
    return path


def main(args: List[str]) -> int:
    parser = argparse.ArgumentParser(description="View loot data from Rogue")
    parser.add_argument("item_db", help="Path to item database")
    parser.add_argument("bin_dir", help="Path to Rogue bin directory")
    pargs = parser.parse_args(args)

    item_db_path = os.path.abspath(pargs.item_db)
    if not os.path.exists(item_db_path):
        print(f"Item database does not exist: {item_db_path}")
        return 1

    bin_dir_path = os.path.abspath(pargs.bin_dir)
    if not os.path.exists(bin_dir_path):
        print(f"Bin directory does not exist: {bin_dir_path}")
        return 1

    loot_info_wrapper_exc = os.path.join(
        bin_dir_path, adapt_exc_path("loot_info")
    )

    item_db = ItemDb.load(item_db_path)

    # Store copy of item db in temp file
    temp_item_db_path = item_db_path + ".tmp.loot_viewer.json"
    item_db.save(temp_item_db_path)

    loot_info_wrapper = LootInfoWrapper(
        item_db_path=temp_item_db_path,
        # FIXME we need to use the build schema here
        schema_path=str(SCHEMAS_PATH / "item_db_schema.json"),
        loot_info_excel_path=loot_info_wrapper_exc,
    )

    loot_viewer = LootViewer(item_db, loot_info_wrapper)
    loot_viewer.setup()
    loot_viewer.run()

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
