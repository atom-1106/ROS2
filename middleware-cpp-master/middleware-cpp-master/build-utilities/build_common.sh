#!/usr/bin/bash
# Copyright (C) Caterpillar Inc. All Rights Reserved.

declare -A AVAILABLE_PLATFORMS=(
    ["arm64"]="tgt_gcc_arm64_guest"
    ["x86"]="tgt_gcc_x86_64_ubuntu_workstation"
    ["all"]="tgt_gcc_arm64_guest tgt_gcc_x86_64_ubuntu_workstation"
)
declare -A AVAILABLE_CONFIGURATIONS=(
    ["debug"]="Debug"
    ["relwithdebinfo"]="RelWithDebInfo"
    ["all"]="Debug RelWithDebInfo"
)
PROJECT_ROOT_DIR="$(realpath "${BASH_SOURCE[0]%/*}/..")"
BUILD_UTILITIES_DIR="$(realpath "${BASH_SOURCE[0]%/*}")"
BUILD_PROFILE="${BUILD_UTILITIES_DIR}/profiles/ubuntu_build.conan"
DOC="https://gitgis.ecorp.cat.com/es-csf-core-info/gen7/bogtrotter_docs/-/blob/master/bogtrotters/releases/middleware_instructions.md"
ALLOWED_CHARACTERS="^[^a-zA-Z0-9.-_]"

export PROJECT_ROOT_DIR
export BUILD_PROFILE
export DOC

readonly ALLOWED_CHARACTERS
readonly BUILD_UTILITIES_DIR
readonly ALLOWED_CHARACTERS

function check_version() {
    local -r VERSION="${1}"

    # Check for special characters, exclude ".", "-", "_"
    [[ "$VERSION" =~ $ALLOWED_CHARACTERS ]] && {
        printf "[Error] Unsupported special characters in version!\n" >&2
        return 1
    }
    # Check for special characters as first character
    [[ "$VERSION" =~ ^[^[:alnum:]] ]] && {
        printf "[Error] First character is a special character!\n" >&2
        return 1
    }
    return 0
}
function check_platform() {
    local -r PLATFORM="${1}"

    # Configuration
    [[ -v AVAILABLE_PLATFORMS[$PLATFORM] ]] || {
        printf "Please specify a target (arm64, x86, all)! Try --help.\n" >&2
        return 1
    }
    return 0
}
function check_build_type() {
    local -r BUILD_TYPE="${1}"

    [[ -v AVAILABLE_CONFIGURATIONS[$BUILD_TYPE] ]] || {
        printf "Please specify a build type (Debug, RelWithDebInfo, All)! Try --help.\n" >&2
        return 1
    }
    return 0
}
function check_system() {
    # Conan
    command -v conan &>/dev/null || {
        printf "conan is missing - are you in the correct docker container?\n" >&2
        return 1
    }
    command -v grep &>/dev/null || {
        printf "This script requires grep!\n" >&2
        return 1
    }
    conan remote list | grep -q "cat-e-conan-center" || {
        printf "This script requires cat-e-conan-center remote!\n" >&2
        return 1
    }
    conan remote list | grep -q "cat-e-conan-rel" || {
        printf "This script requires cat-e-conan-rel remote!\n" >&2
        return 1
    }
    conan remote list | grep -q "cat-e-conan-dev" || {
        printf "This script requires cat-e-conan-dev remote!\n" >&2
        return 1
    }
    return 0
}
function check_requirements() {
    local -r VERSION="${1}"
    local -r PLATFORM="${2}"
    local -r BUILD_TYPE="${3}"
    check_system || return 1
    check_version "${VERSION}" || return 1
    check_platform "${PLATFORM}" || return 1
    check_build_type "${BUILD_TYPE}" || return 1

    return 0
}
