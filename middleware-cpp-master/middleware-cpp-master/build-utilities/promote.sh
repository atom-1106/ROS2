#!/usr/bin/bash

# [NOTE]: This script is temporary until Artifactory Admin can support their own promotion of Middleware.

set -o errexit -o nounset -o pipefail

# Source common
# shellcheck source=/dev/null
source "${BASH_SOURCE[0]%/*}/build_common.sh"

# Variables
VERSION="${1}"
REVISION_ID="${2}"
DOWNLOAD_INFO="downloaded.json"
readonly VERSION
readonly REVISION_ID
readonly DOWNLOAD_INFO

# Required remotes
check_system || exit 1

# Setup
conan remove -c "*" 2>/dev/null || true

# Verify not already release
conan list "cate_middleware/${VERSION}" -r "cat-e-conan-rel" | grep -q "not found" || {
    echo "Package \"cate_middleware/${VERSION}\" was already released!!"
    exit 1
}

# Download package
conan download "cate_middleware/${VERSION}#${REVISION_ID}" -r=cat-e-conan-dev --format=json >"${DOWNLOAD_INFO}"
conan list "cate_middleware/*:*"
echo "Press enter to upload to \"cat-e-conan-rel-local\" or ctrl+c to exit "
IFS= read -r || {
    rm "${DOWNLOAD_INFO}"
    exit 1
}

# Upload to release & clean up
conan upload --list="${DOWNLOAD_INFO}" -r=cat-e-conan-rel -c
rm "${DOWNLOAD_INFO}"
