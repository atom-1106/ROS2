#!/usr/bin/bash
# Copyright (C) Caterpillar Inc. All Rights Reserved.

set -e
cd "${BASH_SOURCE[0]%/*}" || exit 1
MIDDLEWARE_ROOT="$(realpath -m "$PWD/../..")"
readonly MIDDLEWARE_ROOT
readonly TARGET="tgt_gcc_arm64_guest"
# shellcheck disable=SC2015
[ "${1::1}" != "/" ] && [ "${1:1:1}" != "." ] && [ -d "$1" ] || {
	printf "Please specify a folder name whose verity should be built!\n" >&2
	exit 1
}

# always set up conan from scratch in GitLab pipeline
[ "${CI,,}" = "true" ] && rm -f "${CONAN_HOME:-"$HOME/.conan2"}/global.conf"



cd "$1"
mkdir -p "$MIDDLEWARE_ROOT/build/verity/$1"

conan build . \
	-of "$MIDDLEWARE_ROOT/build/verity/$1" \
	-pr:b "$MIDDLEWARE_ROOT/build-utilities/profiles/ubuntu_build.conan" \
	-pr:h "$MIDDLEWARE_ROOT/build-utilities/profiles/host/$TARGET/RelWithDebInfo.conan"
mv "$MIDDLEWARE_ROOT/build/verity/$1/build/verity/"*.verity{,_verify} ../
