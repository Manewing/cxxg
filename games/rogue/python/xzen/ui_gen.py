#!/usr/bin/env python3

import abc
import PySimpleGUI as sg
from functools import partial
from typing import List, Any, Dict, Type, Callable

from xzen.ui import BaseWindow
from xzen.ui import BaseInterface


def convert_snake_case_to_camel_case(s: str) -> str:
    """
    Converts a string from snake_case to camelCase.
    """
    return "".join(word.capitalize() for word in s.split("_"))


def get_title(key: str, obj: Dict[str, Any]) -> str:
    """
    Returns the title for the given object.
    """
    title = obj.get("title")
    if title:
        return title
    return convert_snake_case_to_camel_case(key)


class BaseGeneratedEditor(BaseInterface):
    def __init__(
        self, key: str, obj: Dict[str, Any], prefix: str, parent: BaseInterface
    ):
        title = get_title(key, obj)
        super().__init__(parent=parent, prefix=f"{prefix:s}_{title:s}")
        self.title = title
        self.description = obj.get("description")
        self._on_change_handlers: List[Callable[[Any], None]] = []

    @abc.abstractmethod
    def get_value(self) -> Any:
        pass

    def set_value(
        self, value: Any, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if update_ui:
            self.update_ui()

        if trigger_handlers:
            for handler in self._on_change_handlers:
                handler(value)

    @abc.abstractmethod
    def update_ui(self) -> None:
        pass

    @abc.abstractmethod
    def get_row(self) -> List[sg.Element]:
        pass

    @staticmethod
    def get_type(obj_type: str) -> Type:
        type_map = {
            "string": str,
            "integer": int,
            "number": float,
            "boolean": bool,
        }
        return type_map[obj_type]

    def register_on_change_handler(
        self, handler: Callable[[Any], None]
    ) -> None:
        self._on_change_handlers.append(handler)


class GenerateConstEditor(BaseGeneratedEditor):
    def __init__(
        self, key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.value = obj["const"]

    def get_value(self) -> Any:
        return self.value

    def set_value(
        self, value: Any, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if value != self.value:
            raise ValueError(f"Invalid const value: {value}")
        self.value = value
        super().set_value(value, trigger_handlers, update_ui)

    def update_ui(self) -> None:
        self.w.text.update(f"{self.title:s} {self.value}")

    def get_row(self) -> List[sg.Element]:
        row = [
            sg.Text(
                f"{self.title:s}: {self.value}",
                tooltip=self.description,
                key=self.k.text,
            ),
        ]
        return row


class GeneratedEnumEditor(BaseGeneratedEditor):
    def __init__(
        self, key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.enum_values = sorted(obj["enum"])
        self.value = self.enum_values[0]

    def get_value(self) -> str:
        return self.value

    def set_value(
        self, value: str, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if value not in self.enum_values:
            raise ValueError(f"Invalid enum value: {value}")
        self.value = value
        super().set_value(value, trigger_handlers, update_ui)

    def update_ui(self) -> None:
        self.w.combo.update(self.value)

    def get_row(self) -> List[sg.Element]:
        row = [
            sg.Text(self.title, tooltip=self.description),
            sg.Combo(
                self.enum_values,
                default_value=self.value,
                enable_events=True,
                readonly=True,
                key=self.k.combo,
                tooltip=self.description,
            ),
        ]
        self.register_event(self.k.combo)
        return row

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.combo:
            self.set_value(
                values[self.k.combo], trigger_handlers=True, update_ui=False
            )
            return True
        return False


class GeneratedInputEditor(BaseGeneratedEditor):
    def __init__(
        self, key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.value_type = obj["type"]
        self.value = obj.get("default", self.get_type(self.value_type)())

    def get_value(self) -> Any:
        return self.value

    def set_value(
        self, value: str, trigger_handlers: bool, update_ui: bool
    ) -> None:
        self.value = value
        super().set_value(value, trigger_handlers, update_ui)

    def update_ui(self) -> None:
        self.w.text.update(str(self.value))

    def get_row(self) -> List[sg.Element]:
        row = [
            sg.Text(f"{self.title}: ", tooltip=self.description),
            sg.InputText(
                default_text=str(self.value),
                enable_events=True,
                key=self.k.text,
                tooltip=self.description,
                expand_x=True,
            ),
        ]
        self.register_event(self.k.text)
        return row

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.text:
            value = values[self.k.text]
            try:
                value = self.get_type(self.value_type)(value)
            except ValueError:
                value = self.get_type(self.value_type)()
            self.set_value(value, trigger_handlers=True, update_ui=False)
            return True
        return False


class GeneratedCheckboxEditor(BaseGeneratedEditor):
    def __init__(
        self, key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.value = obj.get("default", False)

    def get_value(self) -> Any:
        return self.value

    def set_value(
        self, value: bool, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if not isinstance(value, bool):
            raise ValueError(f"Expected bool, got: {value}")
        self.value = value
        super().set_value(value, trigger_handlers, update_ui)

    def update_ui(self) -> None:
        self.w.checkbox.update(self.value)

    def get_row(self) -> List[sg.Element]:
        row = [
            sg.Checkbox(
                self.title,
                default=self.value,
                enable_events=True,
                key=self.k.checkbox,
                tooltip=self.description,
            ),
        ]
        self.register_event(self.k.checkbox)
        return row

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.checkbox:
            value = values[self.k.checkbox]
            self.set_value(value, trigger_handlers=True, update_ui=False)
            return True
        return False


class GeneratedColorInputEditor(GeneratedInputEditor):
    def __init__(
        self, key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.value = "#ffffff"

    def get_row(self) -> List[sg.Element]:
        row = super().get_row()
        row += [
            sg.ColorChooserButton("Choose color", target=self.k.text),
        ]
        return row


class GeneratedObjectEditor(BaseGeneratedEditor):
    def __init__(
        self, key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.properties: Dict[str, Any] = obj["properties"]
        self.property_editors: Dict[str, BaseGeneratedEditor] = {}
        for sub_key, sub_value in self.properties.items():
            self.property_editors[sub_key] = create_editor_interface(
                sub_key, sub_value, self, prefix=self.prefix
            )
            self.property_editors[sub_key].register_on_change_handler(
                partial(self._update_property, sub_key)
            )

    def get_value(self) -> Any:
        value = {}
        for sub_key, editor in self.property_editors.items():
            value[sub_key] = editor.get_value()
        return value

    def set_value(
        self, obj: str, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if not isinstance(obj, dict):
            raise ValueError(f"Expected dict, got: {value}")
        super().set_value(obj, trigger_handlers, update_ui=False)

        for key, value in obj.items():
            editor = self.property_editors[key]
            editor.set_value(value, trigger_handlers=False, update_ui=update_ui)

    def update_ui(self) -> None:
        for editor in self.property_editors.values():
            editor.update_ui()

    def _update_property(self, key: str, value: Any) -> None:
        obj = self.get_value()
        obj[key] = value
        self.set_value(obj, trigger_handlers=True, update_ui=False)

    def get_row(self) -> List[sg.Element]:
        if self.description:
            layout = [
                [sg.Text(self.description, size=(40, None))],
                [sg.HSep(pad=(None, 20))],
            ]
        else:
            layout = []
        editors = [v for _, v in sorted(self.property_editors.items())]
        for editor in editors:
            layout += [editor.get_row()]
        return [
            sg.Frame(
                self.title, [[sg.Column(layout, expand_x=True)]], expand_x=True
            )
        ]


class GeneratedArrayEditor(BaseGeneratedEditor):
    def __init__(
        self, key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.item_editor = create_editor_interface(
            key, obj["items"], self, prefix=self.prefix
        )
        self.item_editor.register_on_change_handler(self._update_item)
        self.values: List[Any] = []
        self.selected_idx = 0

    def get_value(self) -> List[Any]:
        return self.values

    def set_value(
        self, value: str, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if not isinstance(value, list):
            raise ValueError(f"Expected list, got: {value}")
        self.values = value
        super().set_value(value, trigger_handlers, update_ui)

    def update_ui(self) -> None:
        self.w.listbox.update(
            [self.get_item_text(idx, x) for idx, x in enumerate(self.values)]
        )
        if self.selected_idx < len(self.values):
            self.w.listbox.update(set_to_index=[self.selected_idx])

    def get_item_text(self, idx: int, item: Any) -> str:
        if isinstance(item, (str, int, float, bool)):
            return f"[{idx}]: {item}"
        title = self.item_editor.title
        return f"[{idx}]: {title}"

    def get_row(self) -> List[sg.Element]:
        toolbar = [
            sg.Text("Search:"),
            sg.InputText(
                "", expand_x=True, key=self.k.search, tooltip="Search"
            ),
            sg.Button("+", key=self.k.add_item, tooltip="Add item"),
            sg.Button("-", key=self.k.rm_item, tooltip="Remove item"),
        ]
        layout = [
            toolbar,
            [
                sg.Listbox(
                    values=[],
                    enable_events=True,
                    size=(40, 10),
                    key=self.k.listbox,
                    tooltip=self.description,
                    expand_x=True,
                ),
            ],
            [sg.HSep(pad=(None, 20))],
            self.item_editor.get_row(),
        ]
        row = [
            sg.Frame(
                self.title,
                [[sg.Column(layout)]],
            )
        ]
        self.register_event(self.k.listbox)
        self.register_event(self.k.add_item)
        self.register_event(self.k.rm_item)
        return row

    def _update_item(self, value: Any) -> None:
        if self.selected_idx >= len(self.values):
            return
        self.values[self.selected_idx] = value
        self.set_value(self.values, trigger_handlers=True, update_ui=True)

    def _update_selected_idx(self, idx: int) -> None:
        if idx >= len(self.values) or idx < 0:
            self.selected_idx = 0
            return
        self.selected_idx = idx
        self.item_editor.set_value(
            self.values[self.selected_idx],
            trigger_handlers=False,
            update_ui=True,
        )

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.listbox:
            self._update_selected_idx(self.w.listbox.get_indexes()[0])
            return True
        if event == self.k.add_item:
            value = self.values + [self.item_editor.get_value()]
            self.selected_idx = len(value) - 1
            self.set_value(
                value,
                trigger_handlers=True,
                update_ui=True,
            )
            return True
        if event == self.k.rm_item:
            if not self.values or self.selected_idx >= len(self.values):
                return True
            self.values.pop(self.selected_idx)
            new_idx = min(self.selected_idx, len(self.values) - 1)
            self._update_selected_idx(new_idx)
            self.set_value(self.values, trigger_handlers=True, update_ui=True)
            return True
        return False


class TypedAnyOfGeneratedEditor(BaseGeneratedEditor):
    def __init__(
        self, key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.any_of = obj.get("anyOf")

        self.editors: Dict[str, GeneratedObjectEditor] = {}
        for obj in self.any_of:
            typed_info = obj["properties"]["type"]["const"]
            self.editors[typed_info] = GeneratedObjectEditor(
                typed_info, obj, self, prefix=f"{prefix:s}_{key:s}"
            )
            self.editors[typed_info].register_on_change_handler(
                partial(self._on_editor_changed, typed_info)
            )
        self.selected_type = list(self.editors.keys())[1]

    def get_types(self) -> List[str]:
        return list(self.editors.keys())

    def get_value(self) -> Any:
        return self.editors[self.selected_type].get_value()

    def set_value(
        self, value: Any, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if not isinstance(value, dict):
            raise ValueError(f"Expected dict, got: {value}")
        if "type" not in value:
            raise ValueError(f"Expected 'type' in dict, got: {value}")
        if value["type"] not in self.editors:
            raise ValueError(f"Invalid type: {value['type']}")
        self.selected_type = value["type"]
        self.editors[self.selected_type].set_value(
            value, trigger_handlers=False, update_ui=update_ui
        )
        super().set_value(value, trigger_handlers, update_ui)

        for typed_info in self.editors:
            enabled = typed_info == self.selected_type
            self.w.get(typed_info).update(visible=enabled)

    def _on_editor_changed(self, typed_info: str, value: Any) -> None:
        self.set_value(value, trigger_handlers=True, update_ui=False)

    def update_ui(self) -> None:
        self.w.combo.update(self.selected_type)
        self.editors[self.selected_type].update_ui()

    def get_row(self) -> List[sg.Element]:
        layout = [
            [
                sg.Text("Any Of", tooltip=self.description),
                sg.Combo(
                    self.get_types(),
                    default_value=self.selected_type,
                    enable_events=True,
                    readonly=True,
                    key=self.k.combo,
                    tooltip=self.description,
                ),
            ],
            [sg.HSep(pad=(None, 20))],
        ]
        for typed_info, editor in self.editors.items():
            enabled = typed_info == self.selected_type
            container = sg.Column(
                [editor.get_row()],
                visible=enabled,
                key=self.k.get(typed_info),
                expand_x=True,
            )
            layout += [[sg.pin(container)]]

        self.register_event(self.k.combo)
        return [sg.Column(layout, expand_x=True)]

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.combo:
            self.selected_type = values[self.k.combo]
            value = self.get_value()
            self.set_value(value, trigger_handlers=True, update_ui=False)
            return True
        return False


def create_string_editor_interface(
    key: str, obj: Dict[str, Any], parent: BaseInterface, prefix: str = ""
) -> BaseGeneratedEditor:
    if "enum" in obj:
        return GeneratedEnumEditor(key, obj, parent, prefix)
    if "pattern" in obj and obj["pattern"] == "#[0-9a-fA-F]{6}":
        return GeneratedColorInputEditor(key, obj, parent, prefix)
    return GeneratedInputEditor(key, obj, parent, prefix)


def _create_typed_editor_interface(
    obj_type: str,
    key: str,
    obj: Dict[str, Any],
    parent: BaseInterface,
    prefix: str = "",
) -> BaseGeneratedEditor:
    handlers = {
        "string": create_string_editor_interface,
        "integer": GeneratedInputEditor,
        "number": GeneratedInputEditor,
        "object": GeneratedObjectEditor,
        "array": GeneratedArrayEditor,
        "boolean": GeneratedCheckboxEditor,
    }
    if obj_type not in handlers:
        raise ValueError(f"/{prefix}/{key}: Unsupported type: {obj_type}")
    return handlers[obj_type](key, obj, parent, prefix)


def create_editor_interface(
    key: Any, obj: Any, parent: BaseInterface, prefix: str = ""
) -> BaseGeneratedEditor:
    if not isinstance(key, str):
        raise ValueError(f"/{prefix}/{key}: Expected str, got: {key}")
    if not isinstance(obj, dict):
        raise ValueError(f"/{prefix}/{key}: Expected dict, got: {obj}")

    if "const" in obj:
        return GenerateConstEditor(key, obj, parent, prefix)

    obj_type = obj.get("type")
    if obj_type:
        return _create_typed_editor_interface(
            obj_type, key, obj, parent, prefix
        )

    any_of = obj.get("anyOf")
    if any_of:
        return TypedAnyOfGeneratedEditor(key, obj, parent, prefix)

    kws = ["type", "anyOf"]
    raise ValueError(
        f"/{prefix}/{key}: Expected one of the keywords {kws} in object, got: {obj}"
    )


class GeneratedJsonEditor(BaseWindow):
    def __init__(self, key: Any, schema_value: Any):
        super().__init__(f"{key}")
        self.key = key
        self.schema_value = schema_value
        self.editor = create_editor_interface(self.key, self.schema_value, self)

    def get_key(self) -> Any:
        return self.key

    def get_value(self) -> Any:
        return self.editor.get_value()

    def get_layout(self) -> List[List[sg.Element]]:
        return [self.editor.get_row()]
