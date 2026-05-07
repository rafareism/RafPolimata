#!/usr/bin/env sh
set -eu
cc="${CC:-gcc}"
mkdir -p build_host_check
ok=0
fail=0
for f in methods/*.c; do
  obj="build_host_check/$(basename "$f" .c).o"
  if "$cc" -std=c11 -Wall -Wextra -Iinclude -c "$f" -o "$obj" >/tmp/rafaelia_cc.log 2>&1; then
    echo "OK   $f"
    ok=$((ok+1))
  else
    echo "SKIP/FAIL $f"
    cat /tmp/rafaelia_cc.log
    fail=$((fail+1))
  fi
done
echo "ok=$ok fail=$fail"
