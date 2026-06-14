#!/bin/bash
A="$HOME/.config/aros"
acc=$(cat "$A/accent" 2>/dev/null || echo "#8B5CF6")
name=$(cat "$A/accent-name" 2>/dev/null || echo purple)
mode=$(cat "$A/mode" 2>/dev/null || echo light)
# waybar accent
if [ -f "$HOME/.config/waybar/style.css.in" ]; then
  sed "s/@ACCENT@/$acc/g" "$HOME/.config/waybar/style.css.in" > "$HOME/.config/waybar/style.css"
  pkill -SIGUSR2 waybar 2>/dev/null
fi
# gtk/gnome accent
gsettings set org.gnome.desktop.interface accent-color "$name" 2>/dev/null
# dark / light
[ -f "$A/wallpaper-light.png" ] || cp "$A/wallpaper.png" "$A/wallpaper-light.png" 2>/dev/null
if [ "$mode" = dark ]; then
  gsettings set org.gnome.desktop.interface color-scheme prefer-dark 2>/dev/null
  cp "$A/wallpaper-dark.png" "$A/wallpaper.png" 2>/dev/null
else
  gsettings set org.gnome.desktop.interface color-scheme default 2>/dev/null
  cp "$A/wallpaper-light.png" "$A/wallpaper.png" 2>/dev/null
fi
pkill swaybg 2>/dev/null; swaybg -i "$A/wallpaper.png" -m fill >/dev/null 2>&1 &
