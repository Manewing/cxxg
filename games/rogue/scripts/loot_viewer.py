#!/usr/bin/env python3

import os
import sys
import json
import argparse
import subprocess
from pathlib import Path

import PySimpleGUI as sg
from matplotlib import pyplot as plt
from typing import List, Optional, Callable, Dict, Any

from xzen.ui import BaseInterface
from xzen.ui import BaseWindow

from xzen.ui_gen import JSONEditorGenerator
from xzen.ui_gen import BaseGeneratedEditor
from xzen.ui_gen import GeneratedArrayEditor


class ItemDb:
    def __init__(self, item_db: dict):
        self.item_db = item_db

    @staticmethod
    def load(item_db_path: str) -> "ItemDb":
        with open(item_db_path, "r") as f:
            item_db = json.load(f)
        return ItemDb(item_db)

    def save(self, item_db_path: str) -> None:
        with open(item_db_path, "w") as f:
            json.dump(self.item_db, f, indent=2, sort_keys=True)

    def get_loot_table_names(self) -> List[str]:
        return sorted(self.item_db["loot_tables"].keys())

    def get_item_names(self) -> List[str]:
        return sorted(x["name"] for x in self.item_db["item_prototypes"])

    def get_loot_table(self, loot_table_name: str) -> dict:
        return self.item_db["loot_tables"][loot_table_name]

    def set_loot_table(self, loot_table_name: str, loot_table: dict) -> None:
        self.item_db["loot_tables"][loot_table_name] = loot_table


class LootInfoWrapper:
    def __init__(self, item_db_path: str, loot_info_excel_path: str):
        self.item_db_path = item_db_path
        self.loot_info_excel_path = loot_info_excel_path

    def get_loot_info(self, loot_table_name: str, rolls: int = 10000) -> dict:
        cmd = [
            self.loot_info_excel_path,
            self.item_db_path,
            "--loot-table",
            loot_table_name,
            str(rolls),
        ]
        output = subprocess.check_output(cmd)
        output = output.decode("utf-8")
        output = output.replace("'", '"')
        try:
            return json.loads(output)
        except json.decoder.JSONDecodeError:
            print(f"Failed to decode JSON:", file=sys.stderr)
            print(output, file=sys.stderr)
            raise


class SelectLootTableViewer(BaseInterface):
    def __init__(
        self,
        parent: BaseInterface,
        item_db: ItemDb,
        loot_info_wrapper: LootInfoWrapper,
        select_cb: Callable[[str], None],
    ):
        super().__init__(parent)
        self.item_db = item_db
        self.select_cb = select_cb
        self.loot_info_wrapper = loot_info_wrapper
        self.loot_tables_column = sg.Column(
            [
                [
                    sg.Text("Loot Tables", justification="center"),
                    sg.InputText(
                        "",
                        size=(20, 1),
                        key=self.k.table_name,
                        enable_events=True,
                    ),
                    sg.Button("+", key=self.k.add_table),
                    sg.Button("-", key=self.k.rm_table),
                ],
                [
                    sg.Listbox(
                        values=self.item_db.get_loot_table_names(),
                        size=(39, 40),
                        key=self.k.loot_tables,
                        enable_events=True,
                    )
                ],
                [
                    sg.Button(
                        "Show Statistics",
                        key=self.k.show_stats,
                    ),
                    sg.VSep(),
                    sg.Button("Save As", key=self.k.save_as),
                ],
            ]
        )

        self.register_event(self.k.table_name)
        self.register_event(self.k.add_table)
        self.register_event(self.k.rm_table)
        self.register_event(self.k.loot_tables)
        self.register_event(self.k.show_stats)
        self.register_event(self.k.save_as)

    def get_element(self) -> sg.Element:
        return self.loot_tables_column

    def _get_loot_table_name(self, values: dict) -> Optional[str]:
        if not values[self.k.loot_tables]:
            sg.popup("Please select a loot table")
            return None
        return values[self.k.loot_tables][0]

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.save_as:
            filename = sg.popup_get_file(
                "",
                save_as=True,
                no_window=True,
                file_types=(("JSON *.json",)),
                initial_folder=os.getcwd(),
            )
            if filename:
                self.item_db.save(filename)
                sg.popup(f"Saved item database to: {filename}")
            return True
        if event == self.k.table_name:
            self.filter_loot_tables(values[self.k.table_name])
            return True
        if event == self.k.add_table:
            self.add_loot_table(self.w.table_name.get())
            return True
        loot_table = self._get_loot_table_name(values)
        if loot_table is None:
            return True
        if event == self.k.rm_table:
            del self.item_db.item_db["loot_tables"][loot_table]
            self.w.loot_tables.update(self.item_db.get_loot_table_names())
            return True
        if event == self.k.loot_tables:
            self.select_cb(loot_table)
            return True
        if event == self.k.show_stats:
            self.view_loot_table(loot_table)
            return True
        return False

    def add_loot_table(self, loot_table_name: str) -> None:
        if loot_table_name in self.item_db.get_loot_table_names():
            sg.popup(f"Loot table already exists: {loot_table_name}")
            return
        loot_table = {
            "rolls": 0,
            "slots": [],
        }
        self.item_db.item_db["loot_tables"][loot_table_name] = loot_table
        self.w.table_name.update("")
        self.w.loot_tables.update(self.item_db.get_loot_table_names())

    def filter_loot_tables(self, filter_str: str) -> None:
        loot_tables = self.item_db.get_loot_table_names()
        loot_tables = [x for x in loot_tables if filter_str in x]
        self.w.loot_tables.update(loot_tables)

    def view_loot_table(self, loot_table_name: str) -> None:
        self.item_db.save(self.loot_info_wrapper.item_db_path)
        loot_info = self.loot_info_wrapper.get_loot_info(loot_table_name)

        # Create bar char of loot table item rewards percentages
        item_names = [x["name"] for x in loot_info["item_rewards"]]
        item_cp_drops = [x["cp_drop"] for x in loot_info["item_rewards"]]
        item_percentages = [x["pp_drop"] for x in loot_info["item_rewards"]]

        fig = plt.figure(figsize=(20, 10))

        ax = fig.add_subplot(111)
        rolls = loot_info["rolls"]
        total = loot_info["total"]
        title = f"Loot Table: {loot_table_name} ({rolls} rolls) ({total} items x{total / rolls:.2f})"
        ax.set_title(title)
        ax.set_xlabel("Item")
        x_pos = [i for i, _ in enumerate(item_names)]
        ax.set_xticks(x_pos)
        ax.set_xticklabels(item_names, rotation=45)

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


class LootSlotsEditor(GeneratedArrayEditor):
    def get_item_text(self, idx: int, slot: Any) -> str:
        prefix = f"({slot['weight']:3d}): "
        if slot["type"] == "item":
            return f"{prefix} Item: {slot['name']}"
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

        self.current_table = None

        schemas_path = Path(__file__).parent.parent / "data" / "schemas"
        schema_files = schemas_path.glob("*_schema.json")
        self.loot_table_path = "https://rogue-todo.com/loot_tb_schema.json"

        self.generator = JSONEditorGenerator.load(schema_files)
        self.generator.register_override(
            self.loot_table_path + "#defs/loot_table/slots",
            LootSlotsEditor
        )
        self.editor: Optional[BaseGeneratedEditor] = None

    def get_layout(self) -> List[List[sg.Element]]:
        self.select_loot_table = SelectLootTableViewer(
            self,
            self.item_db,
            self.loot_info_wrapper,
            self._on_select_loot_table,
        )
        self.editor = self.generator.create_editor_interface_from_path(
            self.loot_table_path + "#defs/loot_table", self
        )
        self.editor.register_on_change_handler(self._on_loot_table_edited)
        return [
            [
                self.select_loot_table.get_element(),
                sg.VSep(),
                sg.Column([self.editor.get_row()]),
            ],
        ]

    def _on_loot_table_edited(self, value: dict) -> None:
        if self.current_table is None:
            return
        self.item_db.set_loot_table(self.current_table, value)

    def _on_select_loot_table(self, loot_table_name: str) -> None:
        self.current_table = loot_table_name
        self.editor.set_value(
            self.item_db.get_loot_table(loot_table_name),
            trigger_handlers=False,
            update_ui=True,
        )


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
        loot_info_excel_path=loot_info_wrapper_exc,
    )

    loot_viewer = LootViewer(item_db, loot_info_wrapper)
    loot_viewer.setup()
    loot_viewer.run()

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
