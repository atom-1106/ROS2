#!/usr/bin/bash
# shellcheck disable=SC2004,SC2034

# run in the middleware-cpp root directory
cd "${BASH_SOURCE[0]%/*}/../.." || exit 1
sa=
build=
create=
tests=
unset DRY_RUN
unset synchronous
usage="Usage: ${0##*/} [OPTIONS] [all|sa|build|create|test]
	--dry-run          do not make lint changes, only print notices and check pass/fail
	--sa-output        folder in which to put static analysis and unfixable linting output
	-s                 run synchronously
	-h, --help         display this help and exit
"
shopt -s extglob
for (( i=1; i <= $#; i++ )); do
	if [ "${!i:0:1}" = "-" ]; then
		if [ "${!i:1:1}" = "-" ]; then
			# long parameters
			case "${!i}" in
				--dry-run) DRY_RUN=; ;;
				--sa-output)
					i=$(($i+1))
					export SA_OUTPUT="${!i}"
					[ ! -e "$SA_OUTPUT" ] || [ -z "$(find "$SA_OUTPUT" -mindepth 1 -maxdepth 1 | head -n 1)" ] || {
						printf "SA_OUTPUT (%s) exists and is not an empty directory!\n" "$SA_OUTPUT" >&2
						exit 1
					}
					# I prefer -n due to pipefail confusion
					# shellcheck disable=SC2143
					[ -n "$(realpath --relative-to="${BASH_SOURCE[0]%/*}" "$SA_OUTPUT" | grep "^../")" ] || {
						printf "SA_OUTPUT (%s) must not be under this directory!\n" "$SA_OUTPUT" >&2
						exit 1
					}
					mkdir -p "$SA_OUTPUT" || {
						printf "Failed to create SA_OUTPUT (%s)!\n" "$SA_OUTPUT" >&2
						exit 1
					}
				;;
				--help)
					printf "%s" "$usage" >&2; exit 0
				;;
				*) printf "%s" "$usage" >&2; exit 2; ;;
			esac
		else
			# short parameters
			case "${!i}" in
				*s*)
					synchronous=
				;;&
				!(-+([sh])))
					printf "Invalid flag: %s\n" "${!i//[sh]/}" >&2
					printf "%s" "$usage" >&2; exit 2
				;;
				*h*)
					printf "%s" "$usage" >&2; exit 0
				;;
			esac
		fi
	else
		case "${!i}" in
			"all") :; ;;
			"sa"|"SA") unset build; unset create; unset tests; ;;
			"build") unset sa; unset create; unset tests; ;;
			"create") unset sa; unset build; unset tests; ;;
			"test") unset sa; unset create; ;;
			*) printf "%s" "$usage" >&2; exit 2; ;;
		esac
		break # don't allow arguments after action
	fi
done

run_in_docker="./external/development-tools/docker/run_in_docker.sh"

job_PIDs=()
declare -A job_files=()
function push_job() {
	stdin=/proc/self/fd/0
	[ -t 0 ] && stdin=/dev/null
	if [ -z "${synchronous+0}" ]; then
		file="$(mktemp)"
		"$1" "${@:2}" < /dev/null &>"$file" &
		job_PIDs+=($!)
		job_files[$!]="$file"
	else
		"$1" "${@:2}" < "$stdin"
	fi
}
function pop_jobs() {
	[ -z "${synchronous+0}" ] || return 0
	for pid in "${job_PIDs[@]}"; do
		tail -f --pid="$pid" -n +0 "${job_files[$pid]}" 2>/dev/null
		rm -f "${job_files[$pid]}"
		# shellcheck disable=SC2184,SC2086
		unset job_files[$pid]
	done
	job_PIDs=()
}

function build() {
	# shellcheck disable=SC2015
	"$run_in_docker" ./build-utilities/build.sh arm64 && \
	"$run_in_docker" ./build-utilities/build.sh x86_64 && \
	true || {
		rm -f "$BUILD_PASSED"; return 1
	}
	return 0;
}

function create() {
	# shellcheck disable=SC2015
	"$run_in_docker" ./build-utilities/create.sh arm64 && \
	"$run_in_docker" ./build-utilities/create.sh x86_64 && \
	true || {
		rm -f "$BUILD_PASSED"; return 1
	}
	return 0;
}

function tests() {
	"$run_in_docker" ./artifacts/tgt_gcc_x86_64_ubuntu_workstation/RelWithDebInfo/tests/MiddlewareTest --gtest_output="xml:/tmp/unit_test_report.xml" || {
		rm -f "$BUILD_PASSED"; return 1
	}
	"$run_in_docker" . mv "/tmp/unit_test_report.xml" ./build/tgt_gcc_x86_64_ubuntu_workstation/RelWithDebInfo/

	"$run_in_docker" ./artifacts/tgt_gcc_x86_64_ubuntu_workstation/RelWithDebInfo/tests/IntegrationTestApp --gtest_output="xml:/tmp/integration_report.xml" || {
		rm -f "$BUILD_PASSED"; return 1
	}
	"$run_in_docker" . mv "/tmp/integration_report.xml" ./build/tgt_gcc_x86_64_ubuntu_workstation/RelWithDebInfo/
}

# shellcheck disable=SC2015
[ -d ./external/development-tools ] && [ -d ./build-utilities ] || {
	printf "Missing development-tools or build-utilities - is this in ssp-info-services?\n" >&2
	exit 1
}
[ "$USER" != "bogbuilder" ] || export DRY_RUN=
# shellcheck disable=SC2155
export BUILD_PASSED="$(mktemp)"
shopt -s nullglob
# shellcheck disable=SC1090
for i in ./external/development-tools/static-analysis/{lint,analysis}/*.sh; do
	source "$i"
done

trap "trap \"\" SIGINT; pop_jobs; exit 1" SIGINT
[ -n "${sa+0}" ] && {
	# can read an array if we have issues with spaces in paths - but we shouldn't
	cppcheck_includes="$(find "./lib" \( -path "*topics*" -prune -o -path "*grpc*" -prune \) -o -type d -print)"
	[ -n "$cppcheck_includes" ] && cppcheck_includes="-I${cppcheck_includes//$'\n'/ -I}"
	push_job lint_newlines
	push_job lint_clang-format
	pop_jobs # lint things may modify files
	push_job analysis_shellcheck
	# shellcheck disable=SC2086
	push_job analysis_cppcheck --check-level=exhaustive --suppress=noExplicitConstructor --suppress=unusedFunction --suppress=missingInclude $cppcheck_includes -DGEN_API_VER=2 -i./idl -i./apps -i./external -i./artifacts -i./build -i./.conan2
	#push_job analysis_flawfinder
}
[ -n "${build+0}" ] && {
	push_job build
}
[ -n "${create+0}" ] && {
	push_job create
}
[ -n "${tests+0}" ] && {
	pop_jobs
	# run in subshell, it starts its own jobs so has its own trap
	(tests)
}

pop_jobs
[ -f "$BUILD_PASSED" ] && { rm "$BUILD_PASSED"; exit 0; } || exit 1
