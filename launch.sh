#!/bin/bash

DB_DIR="database_utils"
DB_PATH="${DB_DIR}/sanet.db"
DB_INIT_SCRIPT="${DB_DIR}/init.sql"

# database setup
rm -rf "$DB_PATH"
sqlite3 "$DB_PATH" < "$DB_INIT_SCRIPT"

# tmux
pkill -f tmux
tmux new-session -d bash
tmux split-window -h bash
#sends keys to first and second terminals
tmux send -t 0:0.0 "python3 flightplanning/flight_server.py" C-m
sleep 15s
tmux send -t 0:0.1 "./robin -v 3" C-m
tmux -2 attach-session -d
