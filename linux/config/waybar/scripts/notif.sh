#!/bin/bash
n=$(makoctl list 2>/dev/null | grep -c '^Notification ')
[ -z "$n" ] && n=0
if [ "$n" -gt 0 ]; then
  printf '{"text":"🔔 %s","class":"has","tooltip":"%s notifications"}\n' "$n" "$n"
else
  printf '{"text":"🔔","class":"none","tooltip":"No notifications"}\n'
fi
