#!/usr/bin/bash
# Copyright (C) Caterpillar Inc. All Rights Reserved.

set -o errexit -o pipefail

BUILD_FOLDER="/tmp/cate_middleware_idl"
conan install . --build=missing -pr:a tgt_gcc_x86_64_ubuntu_tool_release --output-folder "${BUILD_FOLDER}"
conan build . --build=missing -pr:a tgt_gcc_x86_64_ubuntu_tool_release --output-folder "${BUILD_FOLDER}"

sed -i 's|^\s*#\s*error|// cppcheck-suppress preprocessorErrorDirective\n\0|' ./*/*.hpp

rm -rf "${BUILD_FOLDER}"
