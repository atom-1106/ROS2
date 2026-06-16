#!/usr/bin/bash
# Copyright (C) Caterpillar Inc. All Rights Reserved.

set -o errexit -o pipefail

# Source common
# shellcheck source=/dev/null
source "${BASH_SOURCE[0]%/*}/build_common.sh"

# Assign Usage
USAGE="
[Usage]
    ./build.sh -h
    ./build.sh --help
    ./build.sh <package version> x86 debug
    ./build.sh <package version> x86 relwithdebinfo
    ./build.sh <package version> x86 all
    ./build.sh <package version> x86
    ./build.sh <package version> arm64 debug
    ./build.sh <package version> arm64 relwithdebinfo
    ./build.sh <package version> arm64 all
    ./build.sh <package version> arm64
    ./build.sh <package version>
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
DEPLOYED_DIR="/tmp/middleware-cpp"
[ -n "$2" ] && { PLATFORM="${2,,}"; }
[ -n "$3" ] && { BUILD_TYPE="${3,,}"; }

readonly DEPLOYED_DIR

declare -A AVAILABLE_ARCHITECTURES=(
    ["tgt_gcc_arm64_guest"]="armv8"
    ["tgt_gcc_x86_64_ubuntu_workstation"]="x86_64"
)

function prepare() {
    rm -rf "${DEPLOYED_DIR}"
    mkdir -p "${DEPLOYED_DIR}"
}

function clean() {
    local -r TARGET="${1}"
    local -r CONFIGURATION="${2}"
    rm -rf "${PROJECT_ROOT_DIR}/artifacts/${TARGET}/${CONFIGURATION}"
    rm -rf "${PROJECT_ROOT_DIR}/build/${TARGET}/${CONFIGURATION}"
}

function strip_target() {
    local -r DIR="${1}"
    STRIP_EXE="strip"
    if [ "$TARGET" = "tgt_gcc_arm64_guest" ]; then
        ARM64_SDK_BIN_DIR=$(ls -d "${DEPLOYED_DIR}"/full_deploy/build/*sdk_arm64_guest/arm64_guest_os_*/x86_64/bin)
        RESULT_COUNT=$(echo "${ARM64_SDK_BIN_DIR}" | wc -l)
        [ "${RESULT_COUNT}" != 1 ] && {
            echo "Failed due to more than one SDK bin found"
            exit 1
        }
        STRIP_EXE="${ARM64_SDK_BIN_DIR}/aarch64-linux-strip"
        [ ! -f "${ARM64_SDK_BIN_DIR}/aarch64-linux-strip" ] && {
            echo "Missing aarch64-linux-strip"
            exit 1
        }
    fi
    find "${DIR}" -type f -name "*" -exec "${STRIP_EXE}" --strip-debug {} \; 2>/dev/null
}

function install_dependencies() {
    local -r TARGET="${1}"
    local -r CONFIGURATION="${2}"
    local -r CONAN_ARTIFACTS="${PROJECT_ROOT_DIR}/artifacts/${TARGET}/${CONFIGURATION}/conan"

    conan install . --build=missing \
        --version "${VERSION}" \
        -pr:b "${BUILD_PROFILE}" \
        -pr:h "${BUILD_UTILITIES_DIR}/profiles/host/${TARGET}/${CONFIGURATION}.conan" \
        --deployer=full_deploy \
        --deployer-folder="${DEPLOYED_DIR}"

    DEP_LIBRARIES=$(find "${DEPLOYED_DIR}" -type d -path "*/${CONFIGURATION}/${AVAILABLE_ARCHITECTURES[$TARGET]}/lib")
    rm -rf "${CONAN_ARTIFACTS}"
    mkdir -p "${CONAN_ARTIFACTS}"
    for PATHS in $DEP_LIBRARIES; do
        find "${PATHS}" -name "*.so*" -exec cp -a {} "${CONAN_ARTIFACTS}" \;
    done
    # strip dependencies
    strip_target "${CONAN_ARTIFACTS}"
}
function build() {
    local -r TARGET="${1}"
    local -r CONFIGURATION="${2}"

    conan build . \
        --version "${VERSION}" \
        -pr:b "${BUILD_PROFILE}" \
        -pr:h "${BUILD_UTILITIES_DIR}/profiles/host/${TARGET}/${CONFIGURATION}.conan"
}

# Common
check_requirements "${VERSION}" "${PLATFORM}" "${BUILD_TYPE}" || exit 1
prepare

# Build
cd "${PROJECT_ROOT_DIR}" || exit 1
for TARGET in ${AVAILABLE_PLATFORMS["$PLATFORM"]}; do
    for CONFIGURATION in ${AVAILABLE_CONFIGURATIONS["$BUILD_TYPE"]}; do
        clean "${TARGET}" "${CONFIGURATION}"
        install_dependencies "${TARGET}" "${CONFIGURATION}"
        build "${TARGET}" "${CONFIGURATION}"
    done
done

# Final Clean
rm -rf "${DEPLOYED_DIR}" || true
