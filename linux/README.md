# arOS 3 — Linux rice

A themed **Hyprland** (Wayland) desktop that recreates the *arOS 3* concept:
lavender glassmorphism wallpaper, floating frosted pills, a centered round-icon
dock, a fullscreen app-grid launcher, a big-clock lock screen, and glass windows
with real blur, rounded corners, and spring animations.

This is the practical way to get the concept on real hardware — real fonts, real
GPU blur, real apps — instead of the bare-metal build in the repo root.

## What you get

| Concept element | Implemented with |
|---|---|
| Lavender glass wallpaper (light + dark) | `assets/gen-wallpaper.py` → PNG |
| Top search pill "What do you want to do?" | waybar `top` instance |
| Top-right status pill (wifi/vol/clock/battery) | waybar `top` instance |
| Centered round-icon dock (8 apps) | waybar `dock` instance |
| Fullscreen app grid "Good Evening, Abdi!" | wofi (`Super+Space`) |
| Big-clock lock screen over dark glass | swaylock-effects (`Super+L`) |
| Glass windows: blur, rounding, shadow, spring | Hyprland `decoration`/`animations` |
| Notification cards top-right | mako |
| Frosted terminal | kitty |

## Install

On a Linux machine (or VM) running Wayland:

```bash
git clone https://github.com/mohith2309/Ar-os.git
cd Ar-os/linux
./install.sh
```

Then log out and choose **Hyprland**, or run `Hyprland` from a TTY.

Best supported on **Arch** (all packages in repos/AUR). Fedora/Debian install the
core pieces; a few (`swww`, `swaylock-effects`, `hypridle`) may need AUR/COPR/source.

## Keys

- `Super + Space` — app grid launcher
- `Super + Return` — terminal
- `Super + E` — files
- `Super + L` — lock
- `Super + Q` — close window
- `Super + F` — fullscreen
- `Super + drag` — move/resize windows

## Run it in a VM on a Mac

1. Install any Arch-based distro (e.g. EndeavourOS) in UTM/QEMU with 3D accel on.
2. `git clone … && cd Ar-os/linux && ./install.sh`
3. Log into Hyprland.

> Note: GPU blur needs working DRM/3D in the VM. On bare-metal Linux it just works.
