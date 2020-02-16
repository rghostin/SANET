#!/bin/bash
pkill -f tmux

tmux new-session -d bash
tmux split-window -h bash
#sends keys to first and second terminals
tmux send -t 0:0.0 "python3 flightplanning/flight_server.py" C-m
sleep 5s
tmux send -t 0:0.1 "./robin -v 3" C-m
tmux -2 attach-session -d
