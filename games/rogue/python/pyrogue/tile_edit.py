#!/usr/bin/env python3

import PySimpleGUI as sg
from PySimpleGUI.PySimpleGUI import Element
from typing import List, Optional, Dict, Any

from xzen.ui import BaseInterface

from xzen.ui_gen import JSONEditorGenerator
from xzen.ui_gen import GeneratedObjectEditor

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
                            key=self.k.char_disp,
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