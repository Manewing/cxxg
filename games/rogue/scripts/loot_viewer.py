#!/usr/bin/env python3

import os
import re
import sys
import json
import argparse
import subprocess

import PySimpleGUI as sg
from typing import List
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
        return list(self.item_db["loot_tables"].keys())


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


class LootViewer:
    def __init__(self, item_db: ItemDb, loot_info_wrapper: LootInfoWrapper):
        self.item_db = item_db
        self.loot_info_wrapper = loot_info_wrapper
        self.window = None

    def setup(self):
        sg.theme("DarkAmber")
        sg.set_options(font=("Courier New", 16))
        matplotlib.use("tkagg")

        # Create window with a list selection titled "Loot Tables"
        layout = [
            [sg.Text("Loot Tables")],
            [
                sg.Listbox(
                    values=self.item_db.get_loot_table_names(),
                    size=(30, 10),
                    key="-LOOT_TABLES-",
                )
            ],
            [sg.Button("View Loot Table")],
        ]

        self.window = sg.Window("Loot Viewer", layout)

    def run(self):
        while True:
            event, values = self.window.read()
            if event == sg.WIN_CLOSED:
                break
            if event == "View Loot Table":
                loot_table_name = values["-LOOT_TABLES-"][0]
                self.view_loot_table(loot_table_name)

    def view_loot_table(self, loot_table_name: str):
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
        print(loot_info)


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
