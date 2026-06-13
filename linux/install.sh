#!/usr/bin/env bash
# ╭──────────────────────────────────────────────╮
# │  arOS 3 — Hyprland rice installer            │
# ╰──────────────────────────────────────────────╯
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"

pkgs_arch="hyprland waybar wofi swaylock-effects hypridle hyprlock swww mako kitty \
           nautilus pavucontrol fastfetch ttf-inter-variable otf-font-awesome python"

echo "::  arOS 3 rice installer"
echo "::  this themes a Linux box (Wayland) to look like the arOS 3 concept."
echo

if command -v pacman >/dev/null; then
    echo ":: Arch detected — installing packages (needs sudo)…"
    sudo pacman -S --needed --noconfirm $pkgs_arch || \
        echo "!! some pkgs may be in the AUR (swaylock-effects, swww) — install via yay"
elif command -v dnf >/dev/null; then
    echo ":: Fedora detected…"
    sudo dnf install -y hyprland waybar wofi mako kitty nautilus pavucontrol fastfetch \
                        rsms-inter-fonts fontawesome6-fonts python3 || true
    echo "!! swww / swaylock-effects / hypridle: build from source or COPR"
elif command -v apt >/dev/null; then
    echo ":: Debian/Ubuntu detected…"
    sudo apt update
    sudo apt install -y waybar wofi mako-notifier kitty nautilus pavucontrol fastfetch \
                        fonts-inter fonts-font-awesome python3 || true
    echo "!! Hyprland on Debian: use the official packaging or build from source"
else
    echo "!! Unknown distro — install these manually:"
    echo "   $pkgs_arch"
fi

echo
echo ":: generating wallpapers…"
python3 "$HERE/assets/gen-wallpaper.py"

echo ":: copying configs to ~/.config …"
mkdir -p ~/.config/aros
cp -r "$HERE/config/hypr"     ~/.config/
cp -r "$HERE/config/waybar"   ~/.config/
cp -r "$HERE/config/wofi"     ~/.config/
cp -r "$HERE/config/swaylock" ~/.config/
cp -r "$HERE/config/kitty"    ~/.config/
cp -r "$HERE/config/mako"     ~/.config/
cp    "$HERE/config/aros/launcher.sh" ~/.config/aros/
cp    "$HERE/assets/wallpaper.png"      ~/.config/aros/
cp    "$HERE/assets/wallpaper-dark.png" ~/.config/aros/
chmod +x ~/.config/aros/launcher.sh
# swaylock reads /etc-style path; also drop a copy where it looks
mkdir -p ~/.config/swaylock
cp "$HERE/config/swaylock/config" ~/.config/swaylock/config

echo
echo ":: done. Log out and pick 'Hyprland' at your display manager,"
echo "   or run 'Hyprland' from a TTY."
echo "   Super+Space = app grid · Super+Return = terminal · Super+L = lock"
