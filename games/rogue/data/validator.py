#!/usr/bin/env python3

import sys
import json
import argparse
import jsonschema

from typing import List


def validate_json(json_file: str, schema_file: str):
    with open(json_file, 'r') as f:
        json_data = json.load(f)

    with open(schema_file, 'r') as f:
        schema_data = json.load(f)

    jsonschema.validate(json_data, schema_data)


def main(args: List[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('json_file')
    parser.add_argument('schema_file')
    args = parser.parse_args(args[1:])

    try:
        validate_json(args.json_file, args.schema_file)
    except jsonschema.exceptions.ValidationError as e:
        print(e)
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
