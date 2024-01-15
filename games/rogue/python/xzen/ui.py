#!/usr/bin/env python3

import sys
import abc
import matplotlib
import PySimpleGUI as sg
from typing import Optional, Dict, List, Callable, Tuple, Any


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
        self.setups: List[Callable[[], None]] = []
        self.refresh_handlers: List[Callable[[], None]] = []
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

    def handle_setup(self) -> None:
        for setup in self.setups:
            setup()

    def register_setup(self, setup: Callable[[], None]) -> None:
        root = self
        while root.parent is not None:
            root = root.parent
        root.setups.append(setup)

    def handle_refresh(self) -> None:
        for handler in self.refresh_handlers:
            handler()

    def register_refresh_handler(self, handler: Callable[[], None]) -> None:
        root = self
        while root.parent is not None:
            root = root.parent
        root.refresh_handlers.append(handler)

    def trigger_refresh(self) -> None:
        root = self
        while root.parent is not None:
            root = root.parent
        root.needs_refresh = True

    def get_window(self) -> sg.Window:
        if self.parent is not None:
            return self.parent.get_window()
        raise ValueError("Parent not initialized")


class CollapsibleSection(BaseInterface):
    def __init__(
        self,
        layout: List[List[sg.Element]],
        title: str = "",
        arrows: Tuple[str, str] = (sg.SYMBOL_DOWN, sg.SYMBOL_UP),
        collapsed: bool = True,
        parent: Optional["BaseInterface"] = None,
        prefix: str = "",
    ):
        super().__init__(parent, prefix)
        self.layout = layout
        self.title = title
        self.arrows = arrows
        self.collapsed = collapsed

    def get_arrow(self) -> str:
        return self.arrows[0] if self.collapsed else self.arrows[1]

    def get_title(self) -> str:
        arrow = self.get_arrow()
        collapse_str = " (expand)" if self.collapsed else "(collapse)"
        if self.title:
            return f"{arrow} {self.title:s} {collapse_str:s}"
        return f"{arrow} {collapse_str:s}"


    def get_element(self) -> sg.Element:
        col = sg.Column(
            [
                [
                    sg.T(
                        self.get_title(),
                        enable_events=True,
                        key=self.k.collapse_col_btn,
                    ),
                ],
                [
                    sg.pin(
                        sg.Column(
                            self.layout,
                            key=self.k.collapse_col,
                            visible=not self.collapsed,
                        )
                    )
                ],
            ],
            pad=(0, 0),
        )
        self.register_event(self.k.collapse_col_btn)

        return col

    def handle_event(self, event: str, values: dict) -> bool:
        if super().handle_event(event, values):
            return True
        if (
            event == self.k.collapse_col_btn):
            self.collapsed = not self.collapsed
            self.w.collapse_col.update(visible=not self.collapsed)
            self.w.collapse_col_btn.update(self.get_title())
            self.trigger_refresh()
            return True


class ListEditorBase(BaseInterface):
    def __init__(
        self,
        select_cb: Callable[[int], None],
        parent: Optional["BaseInterface"] = None,
        prefix: str = "",
        modifiable: bool = True,
        orderable: bool = True,
        description: Optional[str] = None,
        values: Optional[List[str]] = None,
        size: Tuple[Optional[int], Optional[int]] = (40, 10),
        on_add_item: Optional[Callable[[], Optional[str]]] = None,
        on_rm_item: Optional[Callable[[int], None]] = None,
        on_move_up: Optional[Callable[[int], None]] = None,
        on_move_down: Optional[Callable[[int], None]] = None,
    ):
        super().__init__(parent, prefix)
        self.values: List[str] = [] if values is None else values
        self.selected_idx = 0
        self.filter = ""
        self.modifiable = modifiable
        self.select_cb = select_cb
        self.orderable = orderable
        self.description = description
        self.size = size
        self.on_add_item = on_add_item
        self.on_rm_item = on_rm_item
        self.on_move_up = on_move_up
        self.on_move_down = on_move_down

    def get_values(self) -> List[str]:
        return self.values

    def set_values(
        self, values: List[str], trigger_handlers: bool, update_ui: bool
    ) -> None:
        self.values = values
        self._update_selected_idx(self.selected_idx, trigger_handlers)
        if update_ui:
            self.update_ui()

    def get_real_index(self) -> int:
        filtered_values = self._get_filtered_values()
        if filtered_values:
            return filtered_values[self.selected_idx][0]
        return 0

    def _get_filtered_values(self) -> List[Tuple[int, str, Any]]:
        values = list(enumerate(self.values))
        return [(idx, text) for idx, text in values if self.filter in text]

    def update_ui(self) -> None:
        values = self._get_filtered_values()
        self.w.listbox.update([x for _, x in values])
        if self.selected_idx >= len(values):
            self.selected_idx = len(values) - 1 if len(values) else 0
        if self.selected_idx < len(values):
            self.w.listbox.update(
                set_to_index=[self.selected_idx],
                scroll_to_index=self.selected_idx,
            )

    def get_element(self) -> sg.Element:
        toolbar = [
            sg.Text("Search:"),
            sg.InputText(
                "",
                expand_x=True,
                key=self.k.search,
                enable_events=True,
                tooltip="Search",
            ),
        ]
        self.register_event(self.k.search)
        if self.modifiable:
            toolbar += [
                sg.Button("+", key=self.k.add_item, tooltip="Add item"),
                sg.Button("-", key=self.k.rm_item, tooltip="Remove item"),
            ]
            self.register_event(self.k.add_item)
            self.register_event(self.k.rm_item)

        if self.orderable:
            toolbar += [
                sg.Button("↑", key=self.k.move_up, tooltip="Move up"),
                sg.Button("↓", key=self.k.move_down, tooltip="Move down"),
            ]
            self.register_event(self.k.move_up)
            self.register_event(self.k.move_down)

        layout = [
            toolbar,
            [
                sg.Listbox(
                    values=self.values,
                    enable_events=True,
                    size=self.size,
                    key=self.k.listbox,
                    tooltip=self.description,
                    expand_x=True,
                    expand_y=True,
                ),
            ],
        ]
        self.register_event(self.k.listbox)

        return sg.Column(layout, expand_x=True, expand_y=True)

    def _update_selected_idx(
        self, idx: int, trigger_handlers: bool = True
    ) -> None:
        filtered_values = self._get_filtered_values()
        if idx >= len(filtered_values):
            idx = len(filtered_values) - 1
        if idx < 0:
            idx = 0
        self.selected_idx = idx

        if not filtered_values or not trigger_handlers:
            return
        real_idx, _ = filtered_values[idx]
        self.select_cb(real_idx)

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.listbox:
            indexes = self.w.listbox.get_indexes()
            if not indexes:
                return True
            self._update_selected_idx(indexes[0])
            return True
        if event == self.k.add_item:
            self.handle_add_item()
            return True
        if event == self.k.rm_item:
            self.handle_rm_item()
            return True
        if event == self.k.move_up:
            self.handle_move_up()
            return True
        if event == self.k.move_down:
            self.handle_move_down()
            return True
        if event == self.k.search:
            self.handle_search(values[self.k.search])
            return True
        return False

    def handle_add_item(self) -> None:
        value = "---"
        if self.on_add_item is not None:
            value = self.on_add_item()
        if value is None:
            return
        self.values.append(value)
        self._update_selected_idx(self.selected_idx)
        self.update_ui()

    def handle_rm_item(self) -> None:
        real_idx = self.get_real_index()
        if real_idx >= len(self.values):
            return
        if self.on_rm_item is not None:
            self.on_rm_item(real_idx)
        del self.values[real_idx]
        self._update_selected_idx(self.selected_idx)
        self.update_ui()

    def handle_move_up(self) -> None:
        real_idx = self.get_real_index()
        if real_idx <= 0:
            return
        if self.on_move_up is not None:
            self.on_move_up(real_idx)
        self.values[real_idx - 1], self.values[real_idx] = (
            self.values[real_idx],
            self.values[real_idx - 1],
        )
        self._update_selected_idx(self.selected_idx - 1, True)
        self.update_ui()

    def handle_move_down(self) -> None:
        real_idx = self.get_real_index()
        if real_idx >= len(self.values) - 1:
            return
        if self.on_move_down is not None:
            self.on_move_down(real_idx)
        self.values[real_idx + 1], self.values[real_idx] = (
            self.values[real_idx],
            self.values[real_idx + 1],
        )
        self._update_selected_idx(self.selected_idx + 1, True)
        self.update_ui()

    def handle_search(self, value: str) -> None:
        self.filter = value
        self.update_ui()
        self._update_selected_idx(self.selected_idx)


class BaseWindow(BaseInterface):
    def __init__(self, title: str):
        super().__init__()
        self.title = title
        self.window: Optional[sg.Window] = None
        self.needs_refresh = False

    def setup(self) -> None:
        sg.theme("DarkAmber")
        sg.set_options(font=("Courier New", 16))
        matplotlib.use("tkagg")
        self.window = sg.Window(self.title, self.get_layout(), finalize=True)
        self.handle_setup()

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
            if self.needs_refresh:
                self.needs_refresh = False
                self.handle_refresh()

    def get_window(self) -> sg.Window:
        if self.window is None:
            raise ValueError("Window not initialized")
        return self.window

    def handle_refresh(self) -> None:
        self.get_window().refresh()
        return super().handle_refresh()
