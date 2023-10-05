#!/bin/bash

set -eu -o pipefail

if [ $# -eq 1 ]; then
TIMEOUT=$1
else
TIMEOUT=1
fi

PYTHON3="/usr/bin/env python3"

for example in $(ls data/example_*.json); do
  $PYTHON3 asse.py $example $TIMEOUT
done

echo "SUCCESS"
exit 0
