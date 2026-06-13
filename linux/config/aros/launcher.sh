#!/usr/bin/env bash
# arOS 3 app-grid launcher (toggle)
GREET="Good Evening"
H=$(date +%H)
[ "$H" -lt 12 ] && GREET="Good Morning"
[ "$H" -ge 12 ] && [ "$H" -lt 18 ] && GREET="Good Afternoon"

if pgrep -x wofi >/dev/null; then
    pkill -x wofi
    exit 0
fi

wofi --show drun \
     --conf ~/.config/wofi/config \
     --style ~/.config/wofi/style.css \
     --width 100% --height 100% \
     --columns 4 --allow-images \
     --prompt "$GREET, Abdi!   Search apps…"
