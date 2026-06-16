#!/usr/bin/bash
# Copyright (C) Caterpillar Inc. All Rights Reserved.

set -o errexit -o pipefail

# Source common
# shellcheck source=/dev/null
source "${BASH_SOURCE[0]%/*}/build_common.sh"

# Assign Usage
USAGE="
[Usage]
    ./create.sh -h
    ./create.sh --help
    ./create.sh <package version> x86 debug
    ./create.sh <package version> x86 relwithdebinfo
    ./create.sh <package version> x86 all
    ./create.sh <package version> x86
    ./create.sh <package version> arm64 debug
    ./create.sh <package version> arm64 relwithdebinfo
    ./create.sh <package version> arm64 all
    ./create.sh <package version> arm64
    ./create.sh <package version>

    # Optional: allow dirty repo for conan export
    #   MW_LOCAL_PACKAGE=1 ./create.sh <package version> x86 debug
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

# Assign Arguments
VERSION="${1}"
PLATFORM="all"
BUILD_TYPE="all"
SAVE_ZIP_NAME="conan_cache_save.tgz"
OUTFILE_NAME="package-info.json"

[ -n "$2" ] && { PLATFORM="${2,,}"; }
[ -n "$3" ] && { BUILD_TYPE="${3,,}"; }

readonly SAVE_ZIP_NAME
readonly OUTFILE_NAME

if [[ "${MW_LOCAL_PACKAGE,,}" == "1" || "${MW_LOCAL_PACKAGE,,}" == "true" || "${MW_LOCAL_PACKAGE,,}" == "yes" ]]; then
    export MW_LOCAL_PACKAGE=1
    echo "[Warning] MW_LOCAL_PACKAGE=1: dirty Git repo will be allowed for export."
fi

function prepare() {
    conan remove "cate_middleware" -c 2>/dev/null || true
}

function clean() {
    local -r PACKAGE_ARTIFACTS="${1}"
    conan remove "cate_middleware/${VERSION}" -c 2>/dev/null || true
    rm -rf "${PACKAGE_ARTIFACTS}" || true
    mkdir -p "${PACKAGE_ARTIFACTS}"
}

function install_dependencies() {
    local -r TARGET="${1}"
    local -r CONFIGURATION="${2}"

    conan install . --build=missing \
        --version "${VERSION}" \
        -pr:b "${BUILD_PROFILE}" \
        -pr:h "${BUILD_UTILITIES_DIR}/profiles/host/${TARGET}/${CONFIGURATION}.conan"
}

function create() {
    local -r TARGET="${1}"
    local -r CONFIGURATION="${2}"

    conan create . \
        --version "${VERSION}" \
        -pr:b "${BUILD_PROFILE}" \
        -pr:h "${BUILD_UTILITIES_DIR}/profiles/host/${TARGET}/${CONFIGURATION}.conan"
}

function save() {
    local -r PACKAGE_ARTIFACTS="${1}"
    conan cache save "cate_middleware/${VERSION}:*" \
        --file "${PACKAGE_ARTIFACTS}/${SAVE_ZIP_NAME}" \
        --format json \
        --out-file "${PACKAGE_ARTIFACTS}/${OUTFILE_NAME}"
}

# Common
check_requirements "${VERSION}" "${PLATFORM}" "${BUILD_TYPE}" || exit 1
prepare

# Create
cd "${PROJECT_ROOT_DIR}" || exit 1
for TARGET in ${AVAILABLE_PLATFORMS["$PLATFORM"]}; do
    for CONFIGURATION in ${AVAILABLE_CONFIGURATIONS["$BUILD_TYPE"]}; do
        PACKAGE_ARTIFACTS="${PROJECT_ROOT_DIR}/artifacts/${TARGET}/${CONFIGURATION}/package"
        clean "${PACKAGE_ARTIFACTS}"
        install_dependencies "${TARGET}" "${CONFIGURATION}"
        create "${TARGET}" "${CONFIGURATION}"
        save "${PACKAGE_ARTIFACTS}"
    done
done

# Final Clean
if [[ -z "${MW_LOCAL_PACKAGE}" ]]; then
    conan remove "cate_middleware" -c 2>/dev/null || true
fi
