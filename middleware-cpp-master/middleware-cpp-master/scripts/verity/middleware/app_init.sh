#!/usr/bin/sh
# Copyright (C) Caterpillar Inc. All Rights Reserved.

trap "" PIPE
command -v grep > /dev/null 2>&1 || { printf "The Middleware verity app_init.sh requires grep!\n" >&2; exit 1; }
# shellcheck disable=SC2015
cd "$(dirname "$(realpath "$0")")" 2>/dev/null && [ -f "$(printf "%s\n" ./lib/libMiddleware* | head -n 1)" ] || {
	printf "Failed to find Middleware verity location!\n" >&2
	exit 1
}
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PWD/lib"

# temporary hack - run the Discovery Server on the lowest VM IP
ips="$(grep "|--[[:space:]]*172\." < /proc/net/fib_trie | grep -o "[0-9.]*" | sort | uniq)"
vm_ips="$(grep "^172\." < /etc/hosts | grep -vi -e "[^a-z0-9]host[^a-z0-9]" -e "[^a-z0-9]host$" | grep -o "^[0-9.]*")"
[ -n "$({ printf "%s\n" "$ips"; printf "%s\n" "$vm_ips" | sort -n | head -n 1; } | uniq -d)" ] || {
	printf "This is not the VM with the lowest IP - not running Middleware Discovery Server\n"
	exit 0
}

cleanup() {
	trap "" INT TERM
	pid=$$; children="$({ printf " "; cat /proc/$pid/task/$pid/children; } | tr -s " " | cut -d " " -f 2-)"
	# shellcheck disable=SC2086
	[ -n "$children" ] && kill -9 $children 2>/dev/null # unquoted to split PIDs
	trap - INT TERM
	nohup ./bin/ShmCleanup > /dev/null 2>&1 &
	exit 0
}
trap cleanup INT TERM

while true; do
	printf "Starting Discovery Server\n"
	./bin/DiscoveryServer &
	# waiting (native sh command) so that signals are processed before DiscoveryServer exits
	# allows killing the app_init.sh to kill permanently, or killing the DS to restart it
	wait
done
