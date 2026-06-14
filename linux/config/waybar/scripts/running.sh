#!/bin/bash
if [ "$AROS_FORCE" = "1" ] || hyprctl clients -j 2>/dev/null | grep -qi "\"class\": *\"[^\"]*$1"; then
  echo '{"text":" ","class":"running"}'
else
  echo '{"text":" "}'
fi
