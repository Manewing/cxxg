#!/bin/bash

set -eu -o pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ $# -ne 2 ]; then
    echo "Usage: $0 <build data dir> <source data dir>"
    exit 1
fi

BUILD_DATA_DIR="$1"
SOURCE_DATA_DIR="$2"

$SCRIPT_DIR/tiled_id_map_gen.py  \
    --entity-db $BUILD_DATA_DIR/entity_db.json \
    --tiles-db $SOURCE_DATA_DIR/tiles_db.json \
    --tile-set $SOURCE_DATA_DIR/tiled_maps/tiles.png \
    --tiled-id-map $SOURCE_DATA_DIR/tiled_maps/tiled_id_map.json

