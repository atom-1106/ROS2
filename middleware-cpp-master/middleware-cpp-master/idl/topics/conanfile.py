#!/usr/bin/python3
# Copyright (C) Caterpillar Inc. All Rights Reserved.

import os
import shutil
import textwrap
import conan
from pathlib import Path
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain, CMake
from conan.tools.files import copy
from conan.tools.scm import Version


class ConanProject(conan.ConanFile):
    name = "cate_middleware_idl"
    version = "1.0.0"
    package_type = "application"
    url = "https://gitgis.ecorp.cat.com/es-csf-core-info/info-services/middleware-cpp"
    settings = "arch", "os", "compiler", "build_type", "cat_tgt"
    _idl_dirs = ["message", "requests"]

    def build_requirements(self):
        self.tool_requires("fast-dds-gen/4.1.0")

    def requirements(self):
        self.requires("fast-dds-gen/4.1.0")
        
    def build(self):
        # Command to run fastddsgen
        for idl in self._idl_dirs:
            idl_directory = Path(os.path.join(self.source_folder, idl))
            for file_path in idl_directory.iterdir():
                if file_path.is_file():
                    if file_path.suffix == ".idl":
                        command = f"fastddsgen -replace {file_path.name}"
                        self.output.info(f"Running command: {command}")
                        self.run(command, cwd=file_path.parent)
        

