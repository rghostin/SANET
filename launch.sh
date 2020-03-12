#!/bin/bash

ROOT="/home/ubuntu/sanet"
DB_DIR="$ROOT/database_utils"
DB_PATH="${DB_DIR}/sanet.db"
DB_INIT_SCRIPT="${DB_DIR}/init.sql"
CONF_DIR="$ROOT/conf"
NODEID_CFG="${CONF_DIR}/nodeid.conf"

cd "$ROOT"

mynodeid=$( cat $NODEID_CFG )
echo $mynodeid

# database setup
rm -rf "$DB_PATH"
query="INSERT INTO node_status_map (nodeID, latitude, longitude, timestamp) VALUES ($mynodeid,0,$mynodeid, strftime('%s', 'now'));"
sqlite3 "$DB_PATH" < "$DB_INIT_SCRIPT"
sqlite3 "$DB_PATH" "$query"


# tmux
trap end_servers SIGINT

function end_servers() {
        echo "[*] Trapped CTRL-C"
        tmux kill-session -t 0
        tmuxproc="$(pgrep tmux)"
        if [ -n "$tmuxproc" ] ; then 
            echo "Killing last tmux with PID : $tmuxproc"
            kill "$tmuxproc"
        fi
        exit
}

function tmux_start() {
    tmux new-session -d bash
    tmux split-window -h bash
    #sends keys to first and second terminals
    tmux send -t 0:0.0 "python3 flightplanning/flight_server.py" C-m
    sleep 15s
    tmux send -t 0:0.1 "./robin -v $1" C-m   # todo: fix getopt
    tmux attach-session
}

tmuxproc="$(pgrep tmux)"
echo "Tmux running with PID : $tmuxproc"
pkill -f tmux
if [ -n "$tmuxproc" ] ; then 
    echo "Killing last tmux with PID : $tmuxproc"
    kill "$tmuxproc"
fi

tmux_start 

#mate-terminal --tab --command "tmux attach-session;"  # Only works on NO PC

cat
