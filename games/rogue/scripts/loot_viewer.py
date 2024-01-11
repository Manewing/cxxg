#!/usr/bin/env python3

import os
import abc
import sys
import json
import argparse
import subprocess

import PySimpleGUI as sg
from typing import Any, List, Dict, Optional, Callable
import matplotlib
from matplotlib import pyplot as plt


class ItemDb:
    def __init__(self, item_db: dict):
        self.item_db = item_db

    @staticmethod
    def load(item_db_path: str) -> "ItemDb":
        with open(item_db_path, "r") as f:
            item_db = json.load(f)
        return ItemDb(item_db)

    def get_loot_table_names(self) -> List[str]:
        return sorted(self.item_db["loot_tables"].keys())

    def get_item_names(self) -> List[str]:
        return sorted(x["name"] for x in self.item_db["item_prototypes"])

    def get_loot_table(self, loot_table_name: str) -> dict:
        return self.item_db["loot_tables"][loot_table_name]


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


class KeyGenGenerator:
    def __init__(self, obj: object, prefix: str = ""):
        self.obj = obj
        self.prefix = prefix

    def get(self, name: str) -> str:
        return f"{self.prefix:s}_{self.obj.__class__.__name__:s}_{name:s}"

    def __getattr__(self, name: str) -> str:
        return self.get(name)


class WindowFetcher:
    def __init__(self, window: sg.Window, key_gen: KeyGenGenerator):
        self.window = window
        self.key_gen = key_gen

    def __getattr__(self, name: str) -> sg.Element:
        return self.window[self.key_gen.get(name)]


class BaseInterface(abc.ABC):
    def __init__(self, parent: Optional["BaseInterface"] = None):
        self.parent = parent
        self.events: Dict[str, "BaseInterface"] = {}
        self.window = None

    @property
    def k(self) -> KeyGenGenerator:
        return KeyGenGenerator(self)

    @property
    def w(self) -> WindowFetcher:
        return WindowFetcher(self.get_window(), self.k)

    def handle_event(self, event: str, values: dict) -> bool:
        if event in self.events:
            return self.events[event].handle_event(event, values)
        return False

    def publish_event(self, event: str, values: dict) -> bool:
        if self.parent is None:
            raise ValueError(f"Cannot publish event on root interface: {self}")
        return self.parent.handle_event(event, values)

    def register_event(
        self, event: str, handler: Optional["BaseInterface"] = None
    ) -> None:
        if event in self.events:
            raise ValueError(f"Event already registered: {event}")
        if handler is None:
            handler = self
        if self.parent is not None:
            self.parent.register_event(event, handler)
        else:
            self.events[event] = handler

    def get_window(self) -> sg.Window:
        if self.window is None and self.parent is not None:
            return self.parent.get_window()
        return self.window


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
                [sg.Text("Loot Tables", justification="center")],
                [
                    sg.Listbox(
                        values=self.item_db.get_loot_table_names(),
                        size=(39, 40),
                        key="-LOOT_TABLES-",
                    )
                ],
                [
                    sg.Button("View Loot Table", size=(16, 1)),
                    sg.VSep(),
                    sg.Button("Show Statistics", size=(16, 1)),
                ],
            ]
        )

        self.register_event("View Loot Table")
        self.register_event("Show Statistics")

    def get_element(self) -> sg.Element:
        return self.loot_tables_column

    @staticmethod
    def _get_loot_table_name(values: dict) -> Optional[str]:
        if not values["-LOOT_TABLES-"]:
            sg.popup("Please select a loot table")
            return None
        return values["-LOOT_TABLES-"][0]

    def handle_event(self, event: str, values: dict) -> bool:
        loot_table = self._get_loot_table_name(values)
        if loot_table is None:
            return True
        if event == "View Loot Table":
            self.select_cb(loot_table)
            return True
        if event == "Show Statistics":
            self.view_loot_table(loot_table)
            return True
        return False

    def view_loot_table(self, loot_table_name: str) -> None:
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


class LootTableSlotViewer(BaseInterface):
    def __init__(self, parent: "LootTableViewer", item_db: ItemDb):
        super().__init__(parent)
        self.parent: LootTableViewer = parent
        self.item_db = item_db
        self.slot = None
        self.slot_idx = None
        self.loot_table_name = None
        self.loot_table_slot_column = sg.Column(
            [
                [
                    sg.Text("Type:"),
                    sg.Combo(
                        ["item", "table", "null"],
                        size=(20, 1),
                        key=self.k.tb_type,
                        enable_events=True,
                        readonly=True,
                    ),
                    sg.Button("Save", key=self.k.save),
                ],
                [
                    sg.Text("Weight:"),
                    sg.InputText("", size=(20, 1), key=self.k.weight),
                ],
                [
                    sg.Text("Name:", key=self.k.name_lbl),
                    sg.Combo([], size=(20, 1), key=self.k.name, readonly=True),
                ],
                [
                    sg.Text("Min. Count:", key=self.k.min_count_lbl),
                    sg.InputText("", size=(20, 1), key=self.k.min_count),
                ],
                [
                    sg.Text("Max. Count:", key=self.k.max_count_lbl),
                    sg.InputText("", size=(20, 1), key=self.k.max_count),
                ],
            ],
        )

        self.register_event(self.k.tb_type)
        self.register_event(self.k.save)

    def get_element(self) -> sg.Element:
        return self.loot_table_slot_column

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.save:
            self.save_slot()
            return True
        if event == self.k.tb_type:
            tp = values[self.k.tb_type]
            if tp == "item":
                self._set_item("", 1, 1, 1)
            elif tp == "table":
                self._set_table("", 1)
            elif tp == "null":
                self._set_null(1)
            return True
        return False

    def save_slot(self) -> None:
        if self.slot is None:
            return
        tp = self.w.tb_type.get()
        weight = int(self.w.weight.get())
        if "min_count" in self.slot:
            del self.slot["min_count"]
        if "max_count" in self.slot:
            del self.slot["max_count"]
        if "ref" in self.slot:
            del self.slot["ref"]
        if "name" in self.slot:
            del self.slot["name"]

        self.slot["type"] = tp
        self.slot["weight"] = weight
        if tp == "item":
            name = self.w.name.get()
            min_count = int(self.w.min_count.get())
            max_count = int(self.w.max_count.get())
            self.slot["name"] = name
            self.slot["min_count"] = min_count
            self.slot["max_count"] = max_count
        elif tp == "table":
            ref = self.w.name.get()
            self.slot["ref"] = ref
        elif tp == "null":
            pass

        self.parent.set_table(self.loot_table_name, self.slot_idx)

    def set_slot(self, loot_table_name: str, slot_idx: int) -> None:
        self.slot_idx = slot_idx
        self.loot_table_name = loot_table_name
        loot_table = self.item_db.get_loot_table(loot_table_name)
        self.slot = loot_table["slots"][slot_idx]

        tp = self.slot["type"]
        weight = self.slot["weight"]
        if tp == "item":
            self._set_item(
                self.slot["name"],
                self.slot["min_count"],
                self.slot["max_count"],
                weight,
            )
        elif tp == "table":
            self._set_table(self.slot["ref"], weight)
        elif tp == "null":
            self._set_null(weight)

    def _hide_all(self):
        self.w.name_lbl.update("", visible=False)
        self.w.name.update([], visible=False)
        self.w.min_count.update("", visible=False)
        self.w.max_count.update("", visible=False)
        self.w.min_count_lbl.update(visible=False)
        self.w.max_count_lbl.update(visible=False)

    def _set_item(
        self, name: str, min_count: int, max_count: int, weight: int
    ) -> None:
        # Fill UI with item data
        self.w.tb_type.update("item")
        self.w.name_lbl.update("Name:", visible=True)
        self.w.name.update(
            name, visible=True, values=self.item_db.get_item_names()
        )
        self.w.min_count_lbl.update(visible=True)
        self.w.max_count_lbl.update(visible=True)
        self.w.min_count.update(min_count, visible=True)
        self.w.max_count.update(max_count, visible=True)
        self.w.weight.update(weight)

    def _set_table(self, ref: str, weight: int) -> None:
        # Fill UI with table data
        self._hide_all()
        self.w.tb_type.update("table")
        self.w.name_lbl.update("Ref:", visible=True)
        self.w.name.update(
            ref, visible=True, values=self.item_db.get_loot_table_names()
        )
        self.w.weight.update(weight)

    def _set_null(self, weight: int) -> None:
        # Fill UI with null data
        self._hide_all()
        self.w.tb_type.update("null")
        self.w.weight.update(weight)


class LootTableViewer(BaseInterface):
    def __init__(self, parent: BaseInterface, item_db: ItemDb):
        super().__init__(parent)
        self.item_db = item_db
        self.loot_slot = LootTableSlotViewer(self, item_db)
        self.selected_table = None
        self.loot_table_column = sg.Column(
            [
                [
                    sg.Text("Rolls:"),
                    sg.InputText("", size=(10, 1), key=self.k.rolls),
                ],
                [sg.HSep()],
                [
                    sg.Text("Slots:"),
                    sg.Button("+", key=self.k.add_slot),
                    sg.Button("-", key=self.k.rm_slot),
                ],
                [
                    sg.Listbox(
                        values=[],
                        enable_events=True,
                        size=(39, 15),
                        key=self.k.slots,
                    )
                ],
                [sg.HSep()],
                [self.loot_slot.get_element()],
            ],
        )

        self.register_event(self.k.add_slot)
        self.register_event(self.k.rm_slot)
        self.register_event(self.k.slots)

        self.rolls = 0
        self.slots = []

    def get_element(self) -> sg.Element:
        return self.loot_table_column

    def handle_event(self, event: str, values: dict) -> None:
        if event == self.k.add_slot:
            self.add_slot()
            return True
        if event == self.k.rm_slot:
            self.rm_slot(self.w.slots.get_indexes()[0])
            return True
        if event == self.k.slots:
            self.select_slot(self.w.slots.get_indexes()[0])
            return True
        return False

    def _get_names(self) -> List[str]:
        names = []
        for slot in self.slots:
            prefix = f"({slot['weight']:3d}): "
            if slot["type"] == "item":
                names.append(f"{prefix} Item: {slot['name']}")
            elif slot["type"] == "table":
                names.append(f"{prefix} Table: {slot['ref']}")
            elif slot["type"] == "null":
                names.append(f"{prefix} Null")
        return names

    def add_slot(self) -> None:
        if self.selected_table is None:
            return
        self.slots.append({"type": "null", "weight": -1})
        self.set_table(self.selected_table)
        self.select_slot(len(self.slots) - 1)

    def rm_slot(self, slot_idx: int) -> None:
        if self.selected_table is None:
            return
        del self.slots[slot_idx]
        self.set_table(self.selected_table)
        self.select_slot(len(self.slots) - 1)

    def set_table(
        self, loot_table_name: str, slot_idx: Optional[int] = None
    ) -> None:
        self.selected_table = loot_table_name
        loot_table = self.item_db.get_loot_table(self.selected_table)

        self.rolls = loot_table["rolls"]
        self.slots = loot_table["slots"]

        self.w.rolls.update(self.rolls)
        self.w.slots.update(self._get_names())

        if slot_idx is None:
            slot_idx = 0
        self.select_slot(slot_idx)

    def select_slot(self, slot_idx: int) -> None:
        if not len(self.slots):
            return
        self.loot_slot.set_slot(self.selected_table, slot_idx)
        # Select first slot
        self.w.slots.update(set_to_index=[slot_idx], scroll_to_index=slot_idx)


class LootViewer(BaseInterface):
    def __init__(self, item_db: ItemDb, loot_info_wrapper: LootInfoWrapper):
        super().__init__()
        self.item_db = item_db
        self.loot_info_wrapper = loot_info_wrapper

    def setup(self):
        sg.theme("DarkAmber")
        sg.set_options(font=("Courier New", 16))
        matplotlib.use("tkagg")

        self.select_loot_table = SelectLootTableViewer(
            self,
            self.item_db,
            self.loot_info_wrapper,
            self._on_select_loot_table,
        )
        self.loot_table = LootTableViewer(self, self.item_db)

        # Create window with a list selection titled "Loot Tables"
        layout = [
            [
                self.select_loot_table.get_element(),
                sg.VSep(),
                self.loot_table.get_element(),
            ],
        ]

        self.window = sg.Window("Loot Viewer", layout)

    def _on_select_loot_table(self, loot_table_name: str) -> None:
        self.loot_table.set_table(loot_table_name)

    def run(self):
        while True:
            event, values = self.window.read()
            if event == sg.WIN_CLOSED:
                break
            if not self.handle_event(event, values):
                print(f"Unhandled event: {event}", file=sys.stderr)


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

    item_db = ItemDb.load(pargs.item_db)

    loot_info_wrapper = LootInfoWrapper(
        item_db_path=item_db_path, loot_info_excel_path=loot_info_wrapper_exc
    )

    loot_viewer = LootViewer(item_db, loot_info_wrapper)
    loot_viewer.setup()
    loot_viewer.run()

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
