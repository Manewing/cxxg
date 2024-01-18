#!/usr/bin/env python3

import json
import subprocess
import dataclasses

from pathlib import Path
from typing import List, Optional


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


class ToolError(Exception):
    def __init__(self, tool: str, output: Optional[str], exit_code: int):
        self.tool = tool
        self.output = output
        self.exit_code = exit_code
        msg = f"{tool} failed with {exit_code}"
        if output:
            msg += f" and output:\n{output}"
        else:
            msg += " and produced no output"
        super().__init__(msg)


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
        try:
            output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            output = e.output.decode("utf-8")
            raise ToolError(self.loot_info_excel_path, output, e.returncode)
        output = output.decode("utf-8")
        return output

    def _run_loot_info_exc_json(self, args: List[str]) -> dict:
        output = self._run_loot_info_exc(args)
        output = output.replace("'", '"')
        try:
            return json.loads(output)
        except json.decoder.JSONDecodeError:
            raise ToolError(
                self.loot_info_excel_path,
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
