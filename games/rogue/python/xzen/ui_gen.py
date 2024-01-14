#!/usr/bin/env python3

import os
import abc
import json
import PySimpleGUI as sg
from copy import deepcopy
from functools import partial
from typing import List, Any, Dict, Type, Callable, Optional, Tuple

from xzen.schema import Schema
from xzen.schema import SchemaProcessor

from xzen.ui import BaseWindow
from xzen.ui import BaseInterface
from xzen.ui import ListEditorBase
from xzen.ui import CollapsibleSection


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
        self,
        key: str,
        obj: Dict[str, Any],
        prefix: str,
        parent: Optional[BaseInterface],
    ):
        title = get_title(key, obj)
        super().__init__(parent=parent, prefix=f"{prefix:s}_{title:s}")
        self.title = title
        self.obj = obj
        self.description = obj.get("description")
        self._on_change_handlers: List[Callable[[Any, bool, bool], None]] = []

    def is_required(self) -> bool:
        return not "default" in self.obj

    def needs_store(self):
        if "default" in self.obj:
            return self.obj["default"] != self.get_value()
        return True

    def restore_default(self, trigger_handlers: bool, update_ui: bool) -> bool:
        if not "default" in self.obj:
            return False
        self.set_value(
            self.obj["default"], trigger_handlers, update_ui=update_ui
        )
        return True

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
                handler(value, trigger_handlers, update_ui)

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
        self, handler: Callable[[Any, bool, bool], None]
    ) -> None:
        self._on_change_handlers.append(handler)


class GenerateConstEditor(BaseGeneratedEditor):
    def __init__(
        self,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
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
        self,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
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
            print(f"Invalid enum value: {value}")
            value = self.enum_values[0]
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


class LinkedGeneratedEnumEditor(GeneratedEnumEditor):
    """
    Selectable values of the enum are linked to the value of another JSON file
    or editor
    """

    def __init__(
        self,
        get_enum_values: Callable[[], List[str]],
        generator: "JSONEditorGenerator",
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
    ):
        del generator
        obj["enum"] = get_enum_values()
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.get_enum_values = get_enum_values
        self.value = self.enum_values[0]

    def update_ui(self) -> None:
        self.enum_values = self.get_enum_values()
        self.w.combo.update(self.value, values=self.enum_values)


class NonRequiredEditor(BaseGeneratedEditor):
    """
    Selectable values of the enum are linked to the value of another JSON file
    or editor
    """

    def __init__(
        self,
        editor: BaseGeneratedEditor,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
        present: bool = True,
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.present = present
        self.editor = editor
        self.editor.register_on_change_handler(self._trigger_update)

    def get_value(self) -> Any:
        if self.present:
            return self.editor.get_value()
        return None

    def set_value(
        self, value: Any, trigger_handlers: bool, update_ui: bool
    ) -> None:
        present = True
        self.set_value_and_present(
            value, trigger_handlers, update_ui, set_present=present
        )

    def set_value_and_present(
        self,
        value: Any,
        trigger_handlers: bool,
        update_ui: bool,
        set_present: bool,
    ) -> None:
        present_changed = set_present != self.present
        self.present = set_present
        if self.present:
            self.editor.set_value(
                value, trigger_handlers=False, update_ui=update_ui
            )
        # Force update if present changed, we need to trigger the update since the
        # editor needs to be either visible or not
        if present_changed:
            update_ui = True
        super().set_value(value, trigger_handlers, update_ui)

    def _trigger_update(
        self, value: Any, trigger_handlers: bool, update_ui: bool
    ) -> None:
        self.set_value(
            value,
            trigger_handlers=trigger_handlers,
            update_ui=False,
        )

    def restore_default(self, trigger_handlers: bool, update_ui: bool) -> bool:
        self.editor.restore_default(trigger_handlers, update_ui)
        self.present = False

    def is_required(self) -> bool:
        return self.present

    def get_title(self) -> str:
        sym = sg.SYMBOL_CHECK_SMALL if self.present else sg.SYMBOL_X_SMALL
        return f"{sym} {self.title} (Optional)"

    def get_row(self) -> List[sg.Element]:
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
                            [self.editor.get_row()],
                            key=self.k.collapse_col,
                            visible=self.present,
                        )
                    )
                ],
            ],
            pad=(0, 0),
        )
        self.register_event(self.k.collapse_col_btn)
        self.register_setup(self._handle_update)
        return [col]

    def update_ui(self) -> None:
        self._handle_update()
        super().update_ui()
        self.editor.update_ui()

    def _handle_update(self) -> None:
        self.w.collapse_col_btn.update(self.get_title())
        self.w.collapse_col.update(visible=self.present)
        self.trigger_refresh()

    def handle_event(self, event: str, values: dict) -> bool:
        if event == self.k.collapse_col_btn:
            # Only trigger the handlers that the value changed, UI update
            # is handled separately based on the present state
            self.set_value_and_present(
                self.editor.get_value(),
                trigger_handlers=True,
                update_ui=False,
                set_present=not self.present,
            )
            return True
        return False


class GeneratedInputEditor(BaseGeneratedEditor):
    def __init__(
        self,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
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
        self,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
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
        self,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.value = "#ffffff"

    def get_row(self) -> List[sg.Element]:
        row = super().get_row()
        row += [
            sg.Button(
                "Choose color",
                key=self.k.button,
            )
        ]
        self.register_event(self.k.button)
        return row

    def handle_event(self, event: str, values: dict) -> bool:
        if super().handle_event(event, values):
            return True
        if event == self.k.button:
            color = sg.askcolor(self.value, title=self.title)
            if color:
                self.set_value(color[1], trigger_handlers=True, update_ui=True)
            return True
        return False


class GeneratedObjectEditor(BaseGeneratedEditor):
    def __init__(
        self,
        generator: "JSONEditorGenerator",
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.properties: Dict[str, Any] = obj["properties"]
        self.property_editors: Dict[str, BaseGeneratedEditor] = {}
        for sub_key, sub_value in self.properties.items():
            self.property_editors[sub_key] = generator.create_editor_interface(
                sub_key, sub_value, self, prefix=self.prefix
            )

            # Handle non-required properties
            if not self.is_key_required(sub_key):
                self.property_editors[sub_key] = NonRequiredEditor(
                    self.property_editors[sub_key],
                    sub_key,
                    sub_value,
                    self,
                    prefix="opt_" + self.prefix,
                    present=False,
                )

            self.property_editors[sub_key].register_on_change_handler(
                partial(self._update_property, sub_key)
            )

    def is_key_required(self, key: str) -> bool:
        editor = self.property_editors[key]
        return key in self.obj.get("required", []) and editor.is_required()

    def get_value(self) -> Any:
        value = {}
        for sub_key, editor in self.property_editors.items():
            if not editor.is_required():
                continue
            if editor.needs_store():
                value[sub_key] = editor.get_value()
        return value

    def set_value(
        self, obj: str, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if not isinstance(obj, dict):
            raise ValueError(f"Expected dict, got: {value}")
        for key, editor in self.property_editors.items():
            editor.restore_default(trigger_handlers=False, update_ui=False)
            if key not in obj and self.is_key_required(key):
                obj[key] = editor.get_value()
        for key, value in obj.items():
            editor = self.property_editors[key]
            editor.set_value(value, trigger_handlers=False, update_ui=False)
        self._set_value_no_editors(
            self.get_value(), trigger_handlers, update_ui
        )

    def _set_value_no_editors(
        self, obj: str, trigger_handlers: bool, update_ui: bool
    ) -> None:
        super().set_value(obj, trigger_handlers, update_ui)

    def update_ui(self) -> None:
        for editor in self.property_editors.values():
            editor.update_ui()

    def _update_property(
        self, key: str, value: Any, trigger_handlers: bool, update_ui: bool
    ) -> None:
        obj = self.get_value()
        self._set_value_no_editors(obj, trigger_handlers, update_ui)

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
        self,
        generator: "JSONEditorGenerator",
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.list_ui = ListEditorBase(
            select_cb=self._on_select,
            parent=self,
            prefix=self.prefix,
            description=self.description,
            values=[],
            on_add_item=self._on_add_item,
            on_rm_item=self._on_rm_item,
            on_move_up=self._on_move_up,
            on_move_down=self._on_move_down,
            size=(None, 5),
        )
        self.item_editor = generator.create_editor_interface(
            "item", obj["items"], self, prefix=self.prefix
        )
        self.item_editor.register_on_change_handler(self._update_item)
        self.values: List[Any] = []

    def get_value(self) -> List[Any]:
        return deepcopy(self.values)

    def set_value(
        self, value: Any, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if not isinstance(value, list):
            raise ValueError(f"Expected list, got: {value}")
        self.values = value
        if update_ui:
            values = [
                self.get_item_text(idx) for idx in range(len(self.values))
            ]
            self.list_ui.set_values(
                values, trigger_handlers=True, update_ui=True
            )
            real_idx = self.list_ui.get_real_index()
        else:
            real_idx = 0
        if len(self.values):
            self._on_select(real_idx, update_ui=update_ui)
        super().set_value(value, trigger_handlers, update_ui)

    def update_ui(self) -> None:
        values = [self.get_item_text(idx) for idx in range(len(self.values))]
        self.list_ui.set_values(values, trigger_handlers=False, update_ui=True)

    def get_item_text(self, idx: int) -> str:
        item = self.values[idx]
        if isinstance(item, (str, int, float, bool)):
            return f"[{idx}]: {item}"
        title = self.item_editor.title
        return f"[{idx}]: {title}"

    def get_row(self) -> List[sg.Element]:
        layout_editor = [
            [sg.HSep(pad=(None, 20))],
            self.item_editor.get_row(),
        ]
        self._collapsible_section = CollapsibleSection(
            layout_editor,
            title=self.item_editor.title,
            parent=self,
            prefix=self.prefix,
        )
        layout = [
            [self.list_ui.get_element()],
            [self._collapsible_section.get_element()],
        ]
        row = [
            sg.Frame(
                self.title,
                [[sg.Column(layout)]],
            )
        ]
        return row

    def _update_item(
        self, value: Any, trigger_handlers: bool, update_ui: bool
    ) -> None:
        real_idx = self.list_ui.get_real_index()
        if real_idx >= len(self.values):
            return
        self.values[real_idx] = value
        self.set_value(
            self.values, trigger_handlers=trigger_handlers, update_ui=False
        )
        self.update_ui()

    def _on_select(self, idx: int, update_ui: bool = True) -> None:
        self.item_editor.set_value(
            self.values[idx],
            trigger_handlers=False,
            update_ui=update_ui,
        )

    def _on_add_item(self) -> Optional[str]:
        value = self.item_editor.get_value()
        self.values.append(value)
        self.set_value(self.values, trigger_handlers=True, update_ui=False)
        return self.get_item_text(len(self.values) - 1)

    def _on_rm_item(self, idx: int) -> None:
        self.values.pop(idx)
        self.set_value(self.values, trigger_handlers=True, update_ui=False)

    def _on_move_up(self, idx: int) -> None:
        self.values[idx - 1], self.values[idx] = (
            self.values[idx],
            self.values[idx - 1],
        )
        self.set_value(self.values, trigger_handlers=True, update_ui=False)

    def _on_move_down(self, idx: int) -> None:
        self.values[idx + 1], self.values[idx] = (
            self.values[idx],
            self.values[idx + 1],
        )
        self.set_value(self.values, trigger_handlers=True, update_ui=False)


class TypedAnyOfGeneratedEditor(BaseGeneratedEditor):
    def __init__(
        self,
        generator: "JSONEditorGenerator",
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.any_of = obj.get("anyOf")

        self.editors: Dict[str, GeneratedObjectEditor] = {}
        for obj in self.any_of:
            typed_info = obj["properties"]["type"]["const"]
            self.editors[typed_info] = generator.create_editor_interface(
                typed_info, obj, self, prefix=f"{prefix:s}_{key:s}"
            )
            self.editors[typed_info].register_on_change_handler(
                self._on_editor_changed
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
        self._on_editor_changed(value, trigger_handlers, update_ui)

    def _on_editor_changed(
        self,
        value: Any,
        trigger_handlers: bool,
        update_ui: bool,
    ) -> None:
        super().set_value(value, trigger_handlers, update_ui)

        for typed_info in self.editors:
            enabled = typed_info == self.selected_type
            self.w.get(typed_info).update(visible=enabled)
        self.trigger_refresh()

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


class JSONEditorGenerator:
    @staticmethod
    def create_string_editor_interface(
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str = "",
    ) -> BaseGeneratedEditor:
        if "enum" in obj:
            return GeneratedEnumEditor(key, obj, parent, prefix)
        if "pattern" in obj and obj["pattern"] == "#[0-9a-fA-F]{6}":
            return GeneratedColorInputEditor(key, obj, parent, prefix)
        return GeneratedInputEditor(key, obj, parent, prefix)

    def _create_typed_editor_interface(
        self,
        obj_type: str,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str = "",
    ) -> BaseGeneratedEditor:
        handlers = {
            "string": JSONEditorGenerator.create_string_editor_interface,
            "integer": GeneratedInputEditor,
            "number": GeneratedInputEditor,
            "object": partial(GeneratedObjectEditor, self),
            "array": partial(GeneratedArrayEditor, self),
            "boolean": GeneratedCheckboxEditor,
        }
        if obj_type not in handlers:
            raise ValueError(f"/{prefix}/{key}: Unsupported type: {obj_type}")
        return handlers[obj_type](key, obj, parent, prefix)

    def _create_override(
        self,
        path: str,
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str = "",
    ) -> BaseGeneratedEditor:
        try:
            return self._overrides[path](self, key, obj, parent, prefix)
        except Exception as e:
            raise ValueError(
                f"/{prefix}/{key}: {path} Failed to create override:\n"
                f"{e}\n{obj}"
            ) from e

    def _create_editor_interface_internal(
        self,
        key: Any,
        obj: Any,
        parent: Optional[BaseInterface],
        prefix: str = "",
    ) -> BaseGeneratedEditor:
        print("# Creating editor interface for:", self._current_path)
        if not isinstance(key, str):
            raise ValueError(f"/{prefix}/{key}: Expected str, got: {key}")
        if not isinstance(obj, dict):
            raise ValueError(f"/{prefix}/{key}: Expected dict, got: {obj}")

        if self._current_path in self._overrides:
            print("# Using override for:", self._current_path)
            return self._create_override(
                self._current_path, key, obj, parent, prefix
            )

        if "const" in obj:
            return GenerateConstEditor(key, obj, parent, prefix)

        obj_type = obj.get("type")
        if obj_type:
            return self._create_typed_editor_interface(
                obj_type, key, obj, parent, prefix
            )

        any_of = obj.get("anyOf")
        if any_of:
            return TypedAnyOfGeneratedEditor(self, key, obj, parent, prefix)

        kws = ["type", "anyOf"]
        raise ValueError(
            f"/{prefix}/{key}: Expected one of the keywords {kws} in object, got: {obj}"
        )

    def create_editor_interface(
        self,
        key: Any,
        obj: Any,
        parent: Optional[BaseInterface],
        prefix: str = "",
    ) -> BaseGeneratedEditor:
        path = self._current_path
        try:
            self._current_path = f"{self._current_path:s}/{key:s}"
            return self._create_editor_interface_internal(
                key, obj, parent, prefix
            )
        finally:
            self._current_path = path

    @staticmethod
    def load(schema_files: List[str]) -> "JSONEditorGenerator":
        schemas = []
        for schema_file in schema_files:
            with open(schema_file, "r") as f:
                schema_json = json.load(f)
            schemas.append(Schema(schema_json["$id"], schema_file, schema_json))
        return JSONEditorGenerator(schemas)

    def __init__(self, schemas: List[Schema]):
        self.schemas = schemas
        self.processor = SchemaProcessor(schemas)
        self.processor.resolve_all(remove_keys=False)
        self._overrides: Dict[str, Type] = {}
        self._current_path: str = ""

    def register_override(self, path: str, editor_type: Type) -> None:
        if path in self._overrides:
            raise ValueError(f"Override already registered: {path}")
        self._overrides[path] = editor_type

    def create_editor_interface_from_path(
        self, path: str, parent: Optional[BaseInterface] = None
    ) -> BaseGeneratedEditor:
        self._current_path = SchemaProcessor.normalize_path(path)
        self._current_path = "/".join(self._current_path.split("/")[:-1])
        try:
            ref = self.processor.resolve_external_ref(None, path)
            return self.create_editor_interface(ref.key, ref.value, parent)
        finally:
            self._current_path = ""


class JSONFileManagerInterface(BaseInterface):
    def __init__(
        self,
        title: str,
        on_save: Callable[[str], None] = None,
        on_load: Callable[[str], None] = None,
        path: Optional[str] = None,
        parent: Optional[BaseInterface] = None,
        prefix: str = "",
    ):
        super().__init__(parent=parent, prefix=prefix)
        self._file_path: Optional[str] = path
        self._on_save = on_save
        self._on_load = on_load
        self._title = title

    def get_element(self) -> sg.Element:
        is_save_enabled = self._on_save is not None
        is_load_enabled = self._on_load is not None
        layout = [
            [
                sg.Button(
                    "Load",
                    key=self.k.load,
                    disabled=not is_load_enabled,
                    expand_x=True,
                ),
                sg.Button(
                    "Save",
                    key=self.k.save,
                    disabled=not is_save_enabled,
                    expand_x=True,
                ),
                sg.Button(
                    "Save As",
                    key=self.k.save_as,
                    disabled=not is_save_enabled,
                    expand_x=True,
                ),
            ],
            [sg.InputText(self._file_path, key=self.k.text, expand_x=True)],
        ]
        if is_load_enabled:
            self.register_event(self.k.load)
        if is_save_enabled:
            self.register_event(self.k.save)
            self.register_event(self.k.save_as)
        return sg.Frame(self._title, layout, expand_x=True)

    def _set_file_path(self, path: str) -> None:
        self._file_path = path
        self.w.text.update(path)

    def _save(self) -> None:
        if not self._on_save:
            raise ValueError("Save callback not set")
        self._on_save(self._file_path)
        sg.popup(f"Saved {self._title} database to: {self._file_path}")

    def _handle_save_as(self) -> None:
        initial_folder = os.getcwd()
        if self._file_path:
            initial_folder = os.path.dirname(self._file_path)
        filename = sg.popup_get_file(
            "",
            save_as=True,
            no_window=True,
            file_types=(("JSON *.json",)),
            initial_folder=initial_folder,
        )
        if filename:
            self._set_file_path(filename)
            self._save()

    def _handle_save(self) -> None:
        if not self._file_path:
            self._handle_save_as()
            return
        self._save()

    def _handle_load(self) -> None:
        if not self._on_load:
            raise ValueError("Load callback not set")
        filename = sg.popup_get_file(
            "",
            no_window=True,
            file_types=(("JSON *.json",)),
            initial_folder=os.getcwd(),
        )
        if filename:
            self._set_file_path(filename)
            self._on_load(filename)
            sg.popup(f"Loaded {self._title} from: {filename}")

    def handle_event(self, event: str, values: dict) -> bool:
        if super().handle_event(event, values):
            return True
        if event == self.k.save_as:
            self._handle_save_as()
            return True
        if event == self.k.save:
            self._handle_save()
            return True
        if event == self.k.load:
            self._handle_load()
            return True
        return False


class GeneratedJsonEditor(BaseWindow):
    def __init__(self, editor: BaseGeneratedEditor):
        super().__init__(editor.title)
        self.editor = editor
        self.editor.parent = self

    def get_value(self) -> Any:
        return self.editor.get_value()

    def get_file_tab_layout(self) -> List[List[sg.Element]]:
        self.file_manager = JSONFileManagerInterface(
            "JSON File",
            on_save=self._handle_save,
            on_load=self._handle_load,
            parent=self,
            prefix=self.prefix,
        )
        layout = [[self.file_manager.get_element()]]
        return [[sg.Frame("File", layout, expand_x=True, expand_y=True)]]

    def get_editor_tab_layout(self) -> List[List[sg.Element]]:
        return [self.editor.get_row()]

    def _handle_save(self, path: str) -> None:
        with open(path, "w") as f:
            json.dump(self.get_value(), f, indent=4)

    def _handle_load(self, path: str) -> None:
        with open(path, "r") as f:
            value = json.load(f)
        self.editor.set_value(value, trigger_handlers=True, update_ui=True)

    def get_layout(self) -> List[List[sg.Element]]:
        file_tab_layout = self.get_file_tab_layout()
        file_tab = sg.Tab("File", file_tab_layout)

        editor_tab_layout = self.get_editor_tab_layout()
        editor_tab = sg.Tab("Editor", editor_tab_layout)

        return [[sg.TabGroup([[file_tab, editor_tab]])]]
