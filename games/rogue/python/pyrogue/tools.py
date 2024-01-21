#!/usr/bin/env python3

import os
import sys
import json
import subprocess
import dataclasses
import tempfile

from pathlib import Path
from typing import List, Optional


class TempJsonFile:
    def __init__(self, json_file: Path):
        self.json_file = Path(json_file)
        self.temp_file = None
        with open(json_file, "r") as f:
            self.data = json.load(f)

    def write(self) -> Path:
        self.temp_file = Path(str(self.json_file) + ".tmp.json")
        with open(self.temp_file, "w") as f:
            json.dump(self.data, f, indent=2, sort_keys=True)
        return self.temp_file

    def clean(self) -> None:
        if self.temp_file:
            os.remove(self.temp_file)
            self.temp_file = None

    def __enter__(self) -> Path:
        self.write()
        return self.temp_file

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        self.clean()


@dataclasses.dataclass
class RogueDataPaths:
    data_dir: Path

    def __post_init__(self) -> None:
        self.data_dir = Path(self.data_dir).resolve()
        if not self.data_dir.is_dir():
            raise ValueError(f"{self.data_dir} is not a directory")

    @property
    def item_db(self) -> Path:
        return self.data_dir / "item_db.json"

    @property
    def item_db_schema(self) -> Path:
        return self.data_dir / "schemas" / "item_db_schema.json"

    @property
    def entity_db(self) -> Path:
        return self.data_dir / "entity_db.json"

    @property
    def entity_db_schema(self) -> Path:
        return self.data_dir / "schemas" / "entity_db_schema.json"

    @property
    def game_config(self) -> Path:
        return self.data_dir / "game_config.json"

    @property
    def test_game_config(self) -> Path:
        return self.data_dir / "test_game_config.json"

    def get_level(self, level_name: str) -> Path:
        return self.data_dir / "levels" / f"{level_name}.json"


@dataclasses.dataclass
class RogueToolPaths:
    base_dir: Path

    def __post_init__(self) -> None:
        self.base_dir = Path(self.base_dir).resolve()
        if not self.base_dir.is_dir():
            raise ValueError(f"{self.base_dir} is not a directory")

    @property
    def map_viewer(self) -> Path:
        return self.base_dir / "map_viewer"

    @property
    def loot_info(self) -> Path:
        return self.base_dir / "loot_info"

    @property
    def rogue(self) -> Path:
        return self.base_dir / "rogue"

    @property
    def data(self) -> RogueDataPaths:
        return RogueDataPaths(self.base_dir / "data")


class ToolError(Exception):
    def __init__(self, cmd: List[str], output: Optional[str], exit_code: int):
        self.cmd = cmd
        self.output = output
        self.exit_code = exit_code
        msg = f"{cmd[0]} failed with {exit_code}"
        if output:
            msg += f" and output:\n{output}"
        else:
            msg += " and produced no output"
        msg += f"\n" + " ".join(cmd)
        super().__init__(msg)


class RogueToolWrapper:
    def __init__(self, tool_paths: RogueToolPaths):
        self.tool_paths = tool_paths

    def _run_cmd(self, cmd: List[str]) -> str:
        cmd = [str(x) for x in cmd]
        print(f"# Running: {' '.join(cmd)}", file=sys.stderr)
        try:
            output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            output = e.output.decode("utf-8")
            raise ToolError(cmd, output, e.returncode)
        output = output.decode("utf-8")
        return output

    def _run_cmd_no_wait(self, cmd: List[str]) -> None:
        cmd = [str(x) for x in cmd]
        print(f"# Running: {' '.join(cmd)}", file=sys.stderr)
        try:
            subprocess.Popen(cmd)
        except subprocess.CalledProcessError as e:
            raise ToolError(cmd, None, e.returncode)

    def _run_cmd_in_new_window_linux(self, cmd: str) -> str:
        self._run_cmd_no_wait(["xterm", "-e", cmd])
        return ""

    def _run_cmd_in_new_window_macos(self, cmd: str) -> str:
        self._run_cmd_no_wait(["open", "-W", "-a", "iTerm", cmd])
        return ""

    def _run_cmd_in_new_window(self, cmd: List[str]) -> str:
        cmd = [str(x) for x in cmd]
        temp_sh = tempfile.mktemp(suffix=".sh")
        with open(temp_sh, "w") as f:
            f.write("#!/bin/bash\n")
            f.write(" ".join(cmd))
            f.write("\n")
            f.write("echo Press enter to exit\n")
            f.write("read\n")
            f.write("\n")
        os.chmod(temp_sh, 0o755)

        if sys.platform == "linux":
            return self._run_cmd_in_new_window_linux(temp_sh)
        if sys.platform == "darwin":
            return self._run_cmd_in_new_window_macos(temp_sh)
        raise NotImplementedError(f"Unsupported platform: {sys.platform}")

    def _run_tool(
        self, tool: str, args: List[str], /, *, new_window: bool = False
    ) -> str:
        if new_window:
            return self._run_cmd_in_new_window([tool, *args])
        return self._run_cmd([tool, *args])

    def rogue(self, args: List[str], /, *, new_window: bool = False) -> str:
        return self._run_tool(
            self.tool_paths.rogue, args, new_window=new_window
        )

    def map_viewer(
        self, args: List[str], /, *, new_window: bool = False
    ) -> str:
        return self._run_tool(
            self.tool_paths.map_viewer, args, new_window=new_window
        )

    def loot_info(self, args: List[str], /, *, new_window: bool = False) -> str:
        return self._run_tool(
            self.tool_paths.loot_info, args, new_window=new_window
        )

    def build(self) -> None:
        self._run_cmd(["ninja", "-C", self.tool_paths.base_dir.parent.parent])


class LootInfoWrapper:
    def __init__(
        self, item_db_path: str, schema_path: str, loot_info_excel_path: str
    ):
        self.item_db_path = item_db_path
        self.schema_path = schema_path
        self.loot_info_excel_path = loot_info_excel_path

    def _run_loot_info_exc(self, args: List[str]) -> str:
        cmd = [
            self.loot_info_excel_path,
            self.item_db_path,
            self.schema_path,
            *args,
        ]
        cmd = [str(x) for x in cmd]
        try:
            output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            output = e.output.decode("utf-8")
            raise ToolError(cmd, output, e.returncode)
        output = output.decode("utf-8")
        return output

    def _run_loot_info_exc_json(self, args: List[str]) -> dict:
        output = self._run_loot_info_exc(args)
        output = output.replace("'", '"')
        try:
            return json.loads(output)
        except json.decoder.JSONDecodeError:
            raise ToolError(
                [self.loot_info_excel_path],
                f"Failed to decode loot info JSON: {output}",
                1,
            )

    def get_loot_info(
        self, loot_table_name: str, rolls: int = 10000
    ) -> Optional[dict]:
        return self._run_loot_info_exc_json(
            [
                "--loot-table",
                loot_table_name,
                str(rolls),
            ]
        )

    def roll_for_loot(self, loot_table_name: str) -> Optional[str]:
        return self._run_loot_info_exc(
            ["--loot-table", loot_table_name, str(1)]
        )

    def dump_item(self, item_name: str, rolls: int = 1) -> Optional[str]:
        return self._run_loot_info_exc(
            [
                "--dump-item",
                item_name,
                str(rolls),
            ]
        )
