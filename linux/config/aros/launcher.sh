#!/usr/bin/env bash
if pgrep -f aros-launchpad >/dev/null; then pkill -f aros-launchpad; exit 0; fi
exec env GSK_RENDERER=cairo python3 ~/.config/aros/aros-launchpad.py
