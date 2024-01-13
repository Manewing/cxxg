#!/usr/bin/env python3

import abc
import PySimpleGUI as sg
from typing import Optional, Dict


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
