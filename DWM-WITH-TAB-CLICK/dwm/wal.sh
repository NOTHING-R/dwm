#!/usr/bin/env bash
# ─────────────────────────────────────────────
#  wallpaper-menu.sh
#  Browse wallpapers visually with nsxiv thumbnails,
#  set with feh, generate colorscheme with pywal16.
#
#  Dependencies: nsxiv (or sxiv), feh, pywal16 (wal)
#  Install:  sudo pacman -S nsxiv feh
#            pip install pywal16
#
#  Controls inside nsxiv thumbnail view:
#    Arrow keys   → navigate thumbnails
#    +  /  -      → zoom thumbnails in/out
#    m            → MARK the wallpaper you want
#    q            → quit & APPLY the marked wallpaper
#    Escape       → cancel without applying anything
# ─────────────────────────────────────────────

# ── CONFIG ────────────────────────────────────
WALLPAPER_DIR="${HOME}/my-shared-fiels/hypr_wallpaper"
# Optional extra wal flags, e.g. "--backend colorz" or "-l" for light theme
WAL_OPTS=""
# ─────────────────────────────────────────────

# Prefer nsxiv, fall back to sxiv
if command -v nsxiv &>/dev/null; then
  SXIV="nsxiv"
elif command -v sxiv &>/dev/null; then
  SXIV="sxiv"
else
  notify-send "wallpaper-menu" "nsxiv/sxiv not found. Install: sudo pacman -S nsxiv" 2>/dev/null
  echo "Error: nsxiv or sxiv is not installed." >&2
  exit 1
fi

# Make sure the wallpaper directory exists
if [[ ! -d "$WALLPAPER_DIR" ]]; then
  notify-send "wallpaper-menu" "Directory not found: $WALLPAPER_DIR" 2>/dev/null
  echo "Error: WALLPAPER_DIR '$WALLPAPER_DIR' does not exist." >&2
  exit 1
fi

# Gather image files (common formats)
mapfile -d '' images < <(
  find "$WALLPAPER_DIR" \
    -type f \
    \( -iname "*.jpg" -o -iname "*.jpeg" \
    -o -iname "*.png" -o -iname "*.webp" \
    -o -iname "*.bmp" -o -iname "*.gif" \) \
    -print0 | sort -z
)

if [[ ${#images[@]} -eq 0 ]]; then
  notify-send "wallpaper-menu" "No images found in $WALLPAPER_DIR" 2>/dev/null
  echo "Error: no images found in '$WALLPAPER_DIR'." >&2
  exit 1
fi

# Open nsxiv in thumbnail mode (-t), output marked file path on quit (-o)
# Usage: navigate with arrows, press 'm' to mark, then 'q' to apply
chosen_path=$("$SXIV" -t -o "${images[@]}" 2>/dev/null | head -n 1)

# Exit silently if nothing was marked
[[ -z "$chosen_path" ]] && exit 0

# ── Set wallpaper with feh ────────────────────
feh --no-fehbg --bg-fill "$chosen_path"

# ── Generate colorscheme with pywal16 ─────────
# -n  → skip setting the wallpaper again (feh already did it)
wal -i "$chosen_path" -n $WAL_OPTS

# ── Reload anything that reads wal colors ─────
# Uncomment lines below that apply to your setup:

# xrdb merge ~/.cache/wal/colors.Xresources   # reload Xresources (for st, urxvt, dmenu)
# pkill -USR1 -x alacritty 2>/dev/null         # live-reload alacritty
pkill -USR1 -x kitty 2>/dev/null # live-reload kitty
# dwm-msg quit 2>/dev/null                     # restart dwm (if using restart patch)
# ── Reload open fish terminals ─────────────────────────────────
# Merge wal colors AND your base Xresources (preserves DPI)
xrdb merge ~/.cache/wal/colors.Xresources 2>/dev/null
xrdb merge ~/.Xresources 2>/dev/null

# ── Apply GTK theme for applets ───────────────
ln -sf ~/.cache/wal/colors-gtk2.gtkrc ~/.gtkrc-2.0 2>/dev/null
mkdir -p "${XDG_CONFIG_HOME:-$HOME/.config}/gtk-3.0"
ln -sf ~/.cache/wal/colors-gtk3-dark.css \
  "${XDG_CONFIG_HOME:-$HOME/.config}/gtk-3.0/gtk.css" 2>/dev/null

# Restart tray applets so they pick up the new GTK theme
for app in nm-applet blueman-applet flameshot; do
  pkill -x "$app" 2>/dev/null
done
sleep 0.5
nm-applet &
blueman-applet &
flameshot &

notify-send "wallpaper-menu" "Wallpaper set: $(basename "$chosen_path")" 2>/dev/null
# (exit moved to end of script)

# ── Regenerate dwm colors and recompile ───────
DWM_DIR="${HOME}/dwm" # ← must match gen-colors.sh

# Generate the new colors.h from pywal cache
"${DWM_DIR}/gen-colors.sh"

# Recompile and reinstall dwm
if (cd "$DWM_DIR" && sudo make clean install 2>&1 | tail -3); then
  notify-send "wallpaper-menu" "dwm recompiled with new colors" 2>/dev/null
  # Restart dwm — requires the 'restart' patch or use quit to relogin
  # Uncomment whichever applies to you:
  # kill -HUP "$(pidof dwm)"       # if you use a dwm restart patch
  # dwm-msg quit 2>/dev/null        # if you use dwm-msg patch
  echo "dwm recompiled — log out and back in to apply new bar colors"
else
  notify-send "wallpaper-menu" "dwm recompile FAILED — check terminal" 2>/dev/null
fi

# ── Signal dwmblocks to refresh all blocks ────
# pkill -USR1 dwmblocks 2>/dev/null
# New - more reliable
pkill -9 dwmblocks 2>/dev/null
sleep 0.1
dwmblocks &
disown
