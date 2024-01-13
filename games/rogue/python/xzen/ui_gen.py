#!/usr/bin/env python3

import abc
import json
import PySimpleGUI as sg
from functools import partial
from typing import List, Any, Dict, Type, Callable, Optional, Tuple

from xzen.schema import Schema
from xzen.schema import SchemaProcessor

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
        self._on_change_handlers: List[Callable[[Any], None]] = []

    def is_required(self) -> bool:
        if self.obj.get("required", False):
            return True
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
            sg.ColorChooserButton("Choose color", target=self.k.text),
        ]
        return row


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
            self.property_editors[sub_key].register_on_change_handler(
                partial(self._update_property, sub_key)
            )

    def get_value(self) -> Any:
        value = {}
        for sub_key, editor in self.property_editors.items():
            if editor.needs_store():
                value[sub_key] = editor.get_value()
        return value

    def set_value(
        self, obj: str, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if not isinstance(obj, dict):
            raise ValueError(f"Expected dict, got: {value}")
        for key, editor in self.property_editors.items():
            if key not in obj and editor.is_required():
                raise ValueError(f"Expected required key: {key} in {obj}")
            editor.restore_default(trigger_handlers=False, update_ui=False)
        for key, value in obj.items():
            editor = self.property_editors[key]
            editor.set_value(value, trigger_handlers=False, update_ui=False)
        super().set_value(obj, trigger_handlers, update_ui=update_ui)

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
        self,
        generator: "JSONEditorGenerator",
        key: str,
        obj: Dict[str, Any],
        parent: Optional[BaseInterface],
        prefix: str,
    ):
        super().__init__(key=key, obj=obj, parent=parent, prefix=prefix)
        self.item_editor = generator.create_editor_interface(
            key, obj["items"], self, prefix=self.prefix
        )
        self.item_editor.register_on_change_handler(self._update_item)
        self.values: List[Any] = []
        self.selected_idx = 0
        self.filter = ""

    def get_value(self) -> List[Any]:
        return self.values

    def set_value(
        self, value: str, trigger_handlers: bool, update_ui: bool
    ) -> None:
        if not isinstance(value, list):
            raise ValueError(f"Expected list, got: {value}")
        self.values = value
        self._update_selected_idx(self.selected_idx)
        super().set_value(value, trigger_handlers, update_ui)

    def _get_filtered_values(self) -> List[Tuple[int, str, Any]]:
        values = [
            (idx, self.get_item_text(idx, x), x)
            for idx, x in enumerate(self.values)
        ]
        return [
            (idx, text, value)
            for idx, text, value in values
            if self.filter in text
        ]

    def update_ui(self) -> None:
        values = self._get_filtered_values()
        self.w.listbox.update([x for _, x, _ in values])
        if self.selected_idx >= len(values):
            self.selected_idx = len(values) - 1 if len(values) else 0
        if self.selected_idx < len(values):
            self.w.listbox.update(
                set_to_index=[self.selected_idx],
                scroll_to_index=self.selected_idx,
            )

    def get_item_text(self, idx: int, item: Any) -> str:
        if isinstance(item, (str, int, float, bool)):
            return f"[{idx}]: {item}"
        title = self.item_editor.title
        return f"[{idx}]: {title}"

    def get_row(self) -> List[sg.Element]:
        toolbar = [
            sg.Text("Search:"),
            sg.InputText(
                "",
                expand_x=True,
                key=self.k.search,
                enable_events=True,
                tooltip="Search",
            ),
            sg.Button("+", key=self.k.add_item, tooltip="Add item"),
            sg.Button("-", key=self.k.rm_item, tooltip="Remove item"),
            sg.Button("^", key=self.k.move_up, tooltip="Move up"),
            sg.Button("v", key=self.k.move_down, tooltip="Move down"),
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
        self.register_event(self.k.move_up)
        self.register_event(self.k.move_down)
        self.register_event(self.k.search)
        return row

    def _update_item(self, value: Any) -> None:
        if self.selected_idx >= len(self.values):
            return
        self.values[self.selected_idx] = value
        self.set_value(self.values, trigger_handlers=True, update_ui=True)

    def _update_selected_idx(self, idx: int) -> None:
        values = self._get_filtered_values()
        if idx >= len(values):
            idx = len(values) - 1
        if idx < 0:
            idx = 0
        self.selected_idx = idx
        if not values:
            return
        self.item_editor.set_value(
            values[self.selected_idx][2],
            trigger_handlers=False,
            update_ui=True,
        )

    def handle_event(self, event: str, values: dict) -> bool:
        filt_values = self._get_filtered_values()
        if filt_values:
            real_idx = filt_values[self.selected_idx][0]
        else:
            real_idx = 0

        if event == self.k.listbox:
            self._update_selected_idx(self.w.listbox.get_indexes()[0])
            return True
        if event == self.k.add_item:
            new_values = self.values + [self.item_editor.get_value()]
            self.selected_idx = len(new_values) - 1
            self.set_value(
                new_values,
                trigger_handlers=True,
                update_ui=True,
            )
            return True
        if event == self.k.rm_item:
            if not filt_values or self.selected_idx >= len(filt_values):
                return True
            self.values.pop(real_idx)
            new_idx = max(self.selected_idx, len(filt_values) - 1)
            self._update_selected_idx(new_idx)
            self.set_value(self.values, trigger_handlers=True, update_ui=True)
            return True
        if event == self.k.move_up:
            if real_idx <= 0:
                return True
            self.values[real_idx - 1], self.values[real_idx] = (
                self.values[real_idx],
                self.values[real_idx - 1],
            )
            if (real_idx - 1) in [x[0] for x in filt_values]:
                self._update_selected_idx(self.selected_idx - 1)
            self.set_value(self.values, trigger_handlers=True, update_ui=True)
            return True
        if event == self.k.move_down:
            if real_idx >= len(self.values) - 1:
                return True
            self.values[real_idx + 1], self.values[real_idx] = (
                self.values[real_idx],
                self.values[real_idx + 1],
            )
            if (real_idx + 1) in [x[0] for x in filt_values]:
                self._update_selected_idx(self.selected_idx + 1)
            self.set_value(self.values, trigger_handlers=True, update_ui=True)
            return True
        if event == self.k.search:
            self.filter = values[self.k.search]
            self.update_ui()
            self._update_selected_idx(self.selected_idx)
            return True
        return False


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
            self.editors[typed_info] = GeneratedObjectEditor(
                generator, typed_info, obj, self, prefix=f"{prefix:s}_{key:s}"
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
        if not isinstance(key, str):
            raise ValueError(f"/{prefix}/{key}: Expected str, got: {key}")
        if not isinstance(obj, dict):
            raise ValueError(f"/{prefix}/{key}: Expected dict, got: {obj}")

        if self._current_path in self._overrides:
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


class GeneratedJsonEditor(BaseWindow):
    def __init__(self, editor: BaseGeneratedEditor):
        super().__init__(editor.title)
        self.editor = editor
        self.editor.parent = self

    def get_value(self) -> Any:
        return self.editor.get_value()

    def get_layout(self) -> List[List[sg.Element]]:
        return [self.editor.get_row()]
