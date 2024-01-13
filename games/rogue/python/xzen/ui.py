#!/usr/bin/env python3

import sys
import abc
import matplotlib
import PySimpleGUI as sg
from typing import Optional, Dict, List


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

    def get(self, name: str) -> sg.Element:
        return self.window[self.key_gen.get(name)]

    def __getattr__(self, name: str) -> sg.Element:
        return self.get(name)


class BaseInterface(abc.ABC):
    def __init__(
        self, parent: Optional["BaseInterface"] = None, prefix: str = ""
    ):
        self.parent = parent
        self.events: Dict[str, "BaseInterface"] = {}
        self.prefix = prefix

    @property
    def k(self) -> KeyGenGenerator:
        return KeyGenGenerator(self, prefix=self.prefix)

    @property
    def w(self) -> WindowFetcher:
        return WindowFetcher(self.get_window(), self.k)

    def handle_event(self, event: str, values: dict) -> bool:
        if event in self.events:
            return self.events[event].handle_event(event, values)
        return False

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
        if self.parent is not None:
            return self.parent.get_window()
        raise ValueError("Parent not initialized")


class BaseWindow(BaseInterface):
    def __init__(self, title: str):
        super().__init__()
        self.title = title
        self.window: Optional[sg.Window] = None

    def setup(self) -> None:
        sg.theme("DarkAmber")
        sg.set_options(font=("Courier New", 16))
        matplotlib.use("tkagg")
        self.window = sg.Window(self.title, self.get_layout())

    @abc.abstractmethod
    def get_layout(self) -> List[List[sg.Element]]:
        pass

    def run(self):
        while True:
            event, values = self.window.read()
            if event == sg.WIN_CLOSED:
                self.window.close()
                break
            if not self.handle_event(event, values):
                print(f"Unhandled event: {event}", file=sys.stderr)

    def get_window(self) -> sg.Window:
        if self.window is None:
            raise ValueError("Window not initialized")
        return self.window
