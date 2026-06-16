#!/usr/bin/bash
# Copyright (C) Caterpillar Inc. All Rights Reserved.

set -e
SCRIPT_DIR="$(realpath -m "${BASH_SOURCE[0]%/*}")"
MIDDLEWARE_ROOT="$(realpath -m "$SCRIPT_DIR/../../..")"
VERITY_NAME="zzMiddlewareDevTools"
declare -A ARTIFACTS_DIRS=(
    ["Debug"]="${MIDDLEWARE_ROOT}/artifacts/tgt_gcc_arm64_guest/Debug"
    ["RelWithDebInfo"]="${MIDDLEWARE_ROOT}/artifacts/tgt_gcc_arm64_guest/RelWithDebInfo"
)
readonly MIDDLEWARE_ROOT
readonly ARTIFACTS_DIRS
readonly VERITY_NAME

function get_arm64_strip() {
    local TARGET_DIR="$1"
    local DEPLOY_DIR
    DEPLOY_DIR="$(mktemp -d)"

    # Use Conan to deploy the ARM64 guest SDK to get the aarch64-linux-strip tool
    conan install --build=missing \
        --requires="cate_sdk_arm64_guest/arm64_guest_os_2025.4.0" \
        -pr:b "tgt_gcc_x86_64_ubuntu_tool_release" \
        -pr:h "tgt_gcc_arm64_guest" \
        --deployer=full_deploy \
        --deployer-folder="${DEPLOY_DIR}" \
        --output-folder="${DEPLOY_DIR}"
        
    local SDK_BIN_DIR
    SDK_BIN_DIR=$(find "${DEPLOY_DIR}/full_deploy/build" -type d -path "*sdk_arm64_guest/arm64_guest_os_*/x86_64/bin" 2>/dev/null | head -1)

    # Check if the SDK bin directory was found
    STRIP_EXE="${SDK_BIN_DIR}/aarch64-linux-strip"
    if [[ ! -f "${STRIP_EXE}" ]]; then
        echo "[get_arm64_strip] Failed to find aarch64-linux-strip in: ${SDK_BIN_DIR}" >&2
        rm -rf "${DEPLOY_DIR}"
        return 1
    fi

    # Strip ARM64 binaries in the target directory
    find "${TARGET_DIR}" -type f -exec "${STRIP_EXE}" --strip-debug {} \; 2>/dev/null
    rm -rf "${DEPLOY_DIR}"
}

function create_verity() {
    local CONFIGURATION="$1"
    local SOURCE_DIR="${ARTIFACTS_DIRS[$CONFIGURATION]}"
    local STAGING_DIR="/tmp/verity_stage/$CONFIGURATION"
    local OUTPUT_DIR="${SOURCE_DIR}/verity"

    # Check if source directory exists, skip if it doesn't
    if [[ ! -d "${SOURCE_DIR}" ]]; then
        echo "[create_verity] SOURCE_DIR does not exist: ${SOURCE_DIR}. Skipping ${CONFIGURATION}."
        return
    fi

    # Clean up previous staging and output directories
    rm -rf "${OUTPUT_DIR}"
    mkdir -p "${STAGING_DIR}/lib" "${STAGING_DIR}/bin" "${OUTPUT_DIR}"
    # Copy necessary files to staging directory
    cp -ar "${SOURCE_DIR}"/conan/* "${STAGING_DIR}/lib"
    cp -ar "${SOURCE_DIR}"/lib/* "${STAGING_DIR}/lib"
    cp -ar "${SOURCE_DIR}"/apps/middleware-parameters-tester "${STAGING_DIR}/bin"
    cp "${SCRIPT_DIR}/app_init.sh" "${STAGING_DIR}/"
    cp "${SCRIPT_DIR}/README.md" "${STAGING_DIR}/"

    # Strip ARM64 binaries
    get_arm64_strip "${STAGING_DIR}"
    # Create verity package
    cate_verity --name "${VERITY_NAME}_ARM64_GUEST" \
    --staging "${STAGING_DIR}" \
    --output "${OUTPUT_DIR}"
    # Clean up staging directory
    rm -rf "${STAGING_DIR}"
}

create_verity "Debug"
create_verity "RelWithDebInfo"
