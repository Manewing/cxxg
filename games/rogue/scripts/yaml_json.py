#!/usr/bin/env python3
"""
Converts a YAML file to JSON and vice versa.
"""

import json
import yaml
import sys
import argparse

def yaml_to_json(yaml_file: str, json_file: str) -> None:
    """Converts a YAML file to JSON."""
    with open(yaml_file, 'r') as yaml_file:
        data = yaml.load(yaml_file, Loader=yaml.FullLoader)
    with open(json_file, 'w') as json_file:
        json.dump(data, json_file, indent=2, sort_keys=True)

def json_to_yaml(json_file: str, yaml_file: str) -> None:
    """Converts a JSON file to YAML."""
    with open(json_file, 'r') as json_file:
        data = json.load(json_file)
    with open(yaml_file, 'w') as yaml_file:
        yaml.dump(data, yaml_file, indent=2, sort_keys=True)

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', help='input file')
    parser.add_argument('output', help='output file')
    args = parser.parse_args()

    if args.input.endswith('.yaml'):
        if not args.output.endswith('.json'):
            print('Output file must end with .json')
            return 1
        yaml_to_json(args.input, args.output)
    elif args.input.endswith('.json'):
        if not args.output.endswith('.yaml'):
            print('Output file must end with .yaml')
            return 1
        json_to_yaml(args.input, args.output)

    return 0

if __name__ == '__main__':
    sys.exit(main())