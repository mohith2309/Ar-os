#!/bin/bash
LOG="$HOME/.config/aros/autostart.log"
exec >"$LOG" 2>&1
echo "=== arOS autostart $(date) ==="
swaybg -i "$HOME/.config/aros/wallpaper.png" -m fill &
# first-run setup wizard (OOBE)
if [ ! -f "$HOME/.config/aros/.oobe-done" ]; then
  GSK_RENDERER=cairo python3 "$HOME/.config/aros/aros-oobe.py"
fi
sleep 1
waybar -c "$HOME/.config/waybar/config.jsonc" -s "$HOME/.config/waybar/style.css" &
mako &
echo "launched: $(jobs -p | tr '\n' ' ')"
wait
