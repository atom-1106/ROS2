#!/usr/bin/bash
# Copyright (C) Caterpillar Inc. All Rights Reserved.

set -o errexit -o nounset -o pipefail

# Source common
# shellcheck source=/dev/null
source "${BASH_SOURCE[0]%/*}/build_common.sh"

USAGE="
[Usage]
./upload.sh -h
./upload.sh --help

# Real upload with \"continue\" check
./upload.sh <package version>
# Real upload with no \"continue\" check
./upload.sh <package version> -c
"

# Process arguments
[ -z "$1" ] && {
    echo "[Error] Missing version!!!"
    echo "${USAGE}"
    exit 1
}
[ "$1" = "-h" ] || [ "$1" = "--help" ] && {
    echo "${USAGE}"
    exit 0
}

VERSION="${1}"
CONFIRM=0
OUTPUT_FILE="${PROJECT_ROOT_DIR}/artifacts/package-upload.json"
readonly OUTPUT_FILE

# Arg checks
[ "$#" -ge 2 ] && [ "${2}" = "-c" ] && { CONFIRM=1; }

declare -A EXPECTED_BINARIES=(
    ["armv8 Debug"]="${PROJECT_ROOT_DIR}/artifacts/tgt_gcc_arm64_guest/Debug/package/conan_cache_save.tgz"
    ["armv8 RelWithDebInfo"]="${PROJECT_ROOT_DIR}/artifacts/tgt_gcc_arm64_guest/RelWithDebInfo/package/conan_cache_save.tgz"
    ["x86_64 Debug"]="${PROJECT_ROOT_DIR}/artifacts/tgt_gcc_x86_64_ubuntu_workstation/Debug/package/conan_cache_save.tgz"
    ["x86_64 RelWithDebInfo"]="${PROJECT_ROOT_DIR}/artifacts/tgt_gcc_x86_64_ubuntu_workstation/RelWithDebInfo/package/conan_cache_save.tgz"
)

function prepare() {
    conan remove "cate_middleware" -c 2>/dev/null || true
}

function restore() {
    # Restore Binaries
    for key in "${!EXPECTED_BINARIES[@]}"; do
        echo "Restoring: ${key}"
        PACKAGE="${EXPECTED_BINARIES[$key]}"
        [ -f "${PACKAGE}" ] || {
            echo "Missing ${PACKAGE}!" >&2
            exit 1
        }
        conan cache restore "${PACKAGE}" --out-file /dev/null
    done
}

function upload() {
    conan list "cate_middleware/${VERSION}:*"

    if [ "$CONFIRM" -eq 0 ]; then
        echo "Press enter to upload to \"cat-e-conan-dev-local\" or ctrl+c to exit "
        IFS= read -r || exit 1
    fi

    conan upload "cate_middleware/${VERSION}" -r=cat-e-conan-dev --format json --out-file "${OUTPUT_FILE}"
}

check_system || exit 1
prepare
restore
upload
