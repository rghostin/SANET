#!/bin/bash


DB_DIR="database_utils"
DB_PATH="${DB_DIR}/sanet.db"
DB_INIT_SCRIPT="${DB_DIR}/init.sql"
CONF_DIR="conf"
NODEID_CFG="${CONF_DIR}/nodeid.conf"


mynodeid=$( cat $NODEID_CFG )
echo $mynodeid

# database setup
rm -rf "$DB_PATH"
query="INSERT INTO node_status_map (nodeID, latitude, longitude, timestamp) VALUES ($mynodeid,0,0, strftime('%s', 'now'));"
sqlite3 "$DB_PATH" < "$DB_INIT_SCRIPT"
sqlite3 "$DB_PATH" "$query"