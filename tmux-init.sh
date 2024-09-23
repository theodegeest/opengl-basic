#!/bin/bash

tmux rename-window "code"
tmux new-window -n "run"

tmux select-window -t "code"

clear
