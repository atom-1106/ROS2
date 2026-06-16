#!/usr/bin/bash
# shellcheck disable=SC2004,SC2086,SC2295

# This script lets one easily spin up X lockup publishers or subscribers and, after Y seconds, add or remove Z of them
# It will output a log for each subscriber that sticks around for the entire test
# This log will show how many messages it received for each second
# Unfortunately logging publishers is not possible, after investigation and implementation attempts
#
# One can define whether to use shared memory or UDP as the discovery and transport protocol, and if Z is negative whether the exit should include cleanup
# When Z is negative, they will all exit at once
# When Z is positive, they are spun up as background processes, so it's not instant

# output is never shown for transient participants, but this will show it for those present the whole time
# stderr isn't silenced for anything
readonly SHOW_OUTPUT=0

usage="Usage: ${0##*/} [shm|UDP] [pub|sub] count delta applyDeltaSec runSec [true|false]
	[shm|UDP]     : What communication methods to use
	[pub|sub]     : Whether to run publishers or subscribers
	count         : How many publishers/subscribers to run initially
	delta         : How many publishers/subscribers to add or remove
	applyDeltaSec : How many seconds after startup to apply delta
	runSec        : How many seconds to run in total
	[true|false]  : Whether to exit cleanly - required iff delta < 0
"

# check that sending SIGUSR1 to a new process group works by calling self with pgid-check
# then call self in new process group with `proceed` to continue past this part all set for SIGUSR usage
case "$1" in
	pgid-check)
		coproc bounce {
			trap "" SIGUSR1 # ignore
			while IFS= read -r line; do printf "%s\n" "$line" || break; done
		}
		exec 3>&"${bounce[1]}" # GNU devs don't trust me and coprocess FDs aren't available to subprocesses
		exec 4>&2 2>/dev/null
		{ trap 'printf "0\n"; exit 0' SIGUSR1; while sleep 1; do :; done; } >&3 &
		exec 2>&4 4>&-
		sleep 1
		trap "" SIGUSR1 # ignore
		kill -SIGUSR1 -$$ # negative kills process group
		wait $!
		# shellcheck disable=SC2015
		IFS= read -r -t 1 line <&"${bounce[0]}" && [ "$line" = "0" ] || {
			# shellcheck disable=SC2154
			kill "$bounce_PID"
			exit 1
		}
		exit 0
	;;
	proceed) shift; ;;
	*)
		setsid "${BASH_SOURCE[0]}" pgid-check || { printf "PGID check failed!\n" >&2; exit 1; }
		setsid "${BASH_SOURCE[0]}" proceed "$@"
		exit $?
	;;
esac

# lazy argument validation
set -e; trap 'printf "%s" "$usage"' EXIT
[ "${1#[sS][hH][mM]}${1#[uU][dD][pP]}" != "$1$1" ]
[ "${2#[pP][uU][bB]}${2#[sS][uU][bB]}" != "$2$2" ]
[ -n "$3" ]; [ -z "${3#${3%%[^0-9]*}}" ]
[ -n "$4" ]; [ -z "${4#${4%%[^-0-9]*}}" ]
[ -n "$5" ]; [ -z "${5#${5%%[^0-9]*}}" ]
[ -n "$6" ]; [ -z "${6#${6%%[^0-9]*}}" ]
[ $4 -ge 0 ] || [ "${7,,}" = "true" ] || [ "${7,,}" = "false" ] || { false; }
[ $4 -ge 0 ] || [ $(($3+$4)) -ge 0 ] || { false; } # if delta is negative, applying it to count must not go negative
[ $6 -gt $5 ] # runSec must be longer than applyDeltaSec
set +e; trap - EXIT # clear
COMMUNICATION_METHOD="${1,,}"
readonly COMMUNICATION_METHOD="${COMMUNICATION_METHOD::3}"
PUB_OR_SUB="${2,,}"
readonly PUB_OR_SUB="${PUB_OR_SUB::3}"
readonly COUNT=$3
readonly DELTA=$4
readonly APPLY_DELTA_SEC=$5
readonly RUN_SEC=$6
readonly CLEAN_EXIT=$7

# preparation
readonly PUB_BIN="./LockupPublisherApp"
readonly SUB_BIN="./LockupSubscriberApp"
readonly PUB_CONFIG="../../../../apps/lockup-publisher/config-example.xml"
readonly SUB_CONFIG="../../../../apps/lockup-subscriber/config-example.xml"
# shellcheck disable=SC2015
[ -f "$PUB_BIN" ] && [ -f "$SUB_BIN" ] || {
	printf "This should be run in artifacts/tgt/buildtype/apps!\n" >&2
	exit 1
}
[ "$PUB_OR_SUB" != "pub" ] || [ -f "$PUB_CONFIG" ] || {
	printf "Could not find publisher config!\n" >&2
	exit 1
}
[ "$PUB_OR_SUB" != "sub" ] || [ -f "$SUB_CONFIG" ] || {
	printf "Could not find subscriber config!\n" >&2
	exit 1
}
[ "$PUB_OR_SUB" = "pub" ] && BIN="$PUB_BIN" || BIN="$SUB_BIN"
readonly BIN
[ "$PUB_OR_SUB" = "pub" ] && ARGS=(1 "$PUB_CONFIG") || ARGS=("$SUB_CONFIG")
readonly ARGS

# actual execution
# basically just using SIGUSR2 as SIGINT, but having it be trapped by what should not exit immediately (and not conflict with manual ctrl+c)
# shellcheck disable=SC2064
trap "trap \"\" SIGUSR2; kill -SIGUSR2 -$$; wait; sleep 1" EXIT
trap "exit 0" SIGINT
export COMMUNICATION_METHOD
export EXIT_SIGUSR1="false"
export CLEAN_EXIT

[ $DELTA -lt 0 ] && n=$(($COUNT+$DELTA)) || n=$COUNT
for (( i=0; i<$n; i++ )); do
	{
		{
			"$BIN" "${ARGS[@]}" &
			trap "" SIGUSR1 SIGUSR2 # ignore
			while sleep 0.5; kill -0 $! 2>/dev/null; do
				[ "$PUB_OR_SUB" = "sub" ] && printf "\n"
			done
		} | {
			trap "" SIGUSR1 SIGUSR2 # ignore
			[ "$PUB_OR_SUB" = "pub" ] && {
				if [ "$SHOW_OUTPUT" = "1" ]; then
					grep -v "^$"
				else
					cat > /dev/null
				fi
				exit 0
			}
			count=0
			SECONDS=0
			SECONDS_last="$SECONDS"
			mkdir -p ./lockup-logs
			while IFS= read -r line; do
				[ "$SECONDS" = "$SECONDS_last" ] || {
					printf "%d %d\n" "$SECONDS_last" "$count"
					count=0
					SECONDS_last="$SECONDS"
				}
				[ "$SHOW_OUTPUT" = "1" ] && [ -n "$line" ] && printf "%s\n" "$line" >&2
				[ "${line#*\] received \[}" != "$line" ] || continue
				count=$(($count+1))
			done > ./lockup-logs/$i
		}
	} &
done
export EXIT_SIGUSR1="true"
if [ $DELTA -lt 0 ]; then
	n=${DELTA#-}
	for (( i=0; i<$n; i++ )); do
		"$BIN" "${ARGS[@]}" > /dev/null &
	done
fi

sleep $APPLY_DELTA_SEC || exit 1

if [ $DELTA -lt 0 ]; then
	trap "" SIGUSR1 # ignore
	kill -SIGUSR1 -$$
else
	n=$DELTA
	for (( i=0; i<$n; i++ )); do
		"$BIN" "${ARGS[@]}" > /dev/null &
	done
fi

sleep $(($RUN_SEC-$APPLY_DELTA_SEC)) || exit 1

printf "Lockup testing complete, exiting\n"
# see above EXIT trap
