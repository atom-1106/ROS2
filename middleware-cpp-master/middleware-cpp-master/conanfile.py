#!/usr/bin/python3
# Copyright (C) Caterpillar Inc. All Rights Reserved.

import os
from os import path
import shutil
import textwrap
import conan
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain, CMake
from conan.tools.files import copy
from cate_conan_git import CatEGit
from cate_conan_validate import CatEConanCheck

class ConanProject(conan.ConanFile):
    name = "cate_middleware"
    package_type = "shared-library"
    git_repo_url = "https://gitgis.ecorp.cat.com/es-csf-core-info/info-services/middleware-cpp"
    settings = "arch", "os", "compiler", "build_type", "cat_tgt"

    def var(self, variable):
        return self.buildenv.vars(self).get(variable)

    def export(self):
      categit = CatEGit(self)
      categit.coordinates_to_conandata()

    def source(self):
      categit = CatEGit(self)
      categit.fetch_commit_from_conandata_coordinates()
      categit.run("submodule update --init --recursive")

    def build_requirements(self):
        self.tool_requires("protobuf/<host_version>")

    def requirements(self):
        default_traits = {"headers": True, "libs": True, "transitive_libs": True}
        # Transitive dependencies
        self.requires("abseil/20250127.0", **default_traits)
        self.requires("bzip2/1.0.8", **default_traits)
        
        self.requires("tinyxml2/10.0.0", **default_traits)
        self.requires("asio/1.31.0", **default_traits)
        self.requires("fast-cdr/2.3.0", **default_traits)
        self.requires("foonathan-memory/0.7.3", **default_traits)
        
        
        # Target dependencies
        self.requires("fast-dds/3.4.0", **default_traits)
        self.requires("pugixml/1.14", **default_traits)
        self.requires("boost/1.87.0", **default_traits)
        self.requires(
            "protobuf/5.27.0", **default_traits, run=True, transitive_headers=True
        )
        if self.settings.arch == "x86_64":
            self.requires("gtest/1.15.0", headers=True, libs=True, test=True)

    def layout(self):
        build_path = path.join(
            "build", self.settings.cat_tgt.value, self.settings.build_type.value
        )
        shutil.rmtree(build_path, ignore_errors=True)
        generators_path = path.join(
            build_path,
            "generators",
        )
        cmake_layout(self, build_folder=build_path)
        # cmake_layout overwrites self.folders by appending additional settings to file paths, so we re-assign it after
        # but before we call generate, build, or install
        self.folders.build = build_path
        self.folders.generators = generators_path

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        for key in self.buildenv.vars(self).keys():
            tc.variables[key] = self.var(key)
        tc.variables["EXTERNAL_ROOT"] = path.join(
            self.folders.source_folder, "external"
        )
        tc.variables["CONAN_PACKAGE"] = (
            "ON" if self.folders.package_folder is not None else "OFF"
        )
        tc.variables["BUILD_TARGET"] = self.settings.cat_tgt.value
        # Change the presets so that CMake tools in visual studio code can differentiate between our builds
        tc.presets_prefix = ""
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        # Validate CAT E requirements before packaging
        checker = CatEConanCheck(self)
        checker.validate()        
        # Install the built files to the package folder
        cmake = CMake(self)
        cmake.install()

        artifacts = path.join(self.folders.source_folder, "artifacts", self.settings.cat_tgt.value, self.settings.build_type.value)
        copy(self, pattern="include/protobuf/*", src=artifacts, dst=self.package_folder)
        copy(
            self, pattern="include/framework/*", src=artifacts, dst=self.package_folder
        )
        copy(
            self, pattern="include/parameters/*", src=artifacts, dst=self.package_folder
        )
        copy(
            self,
            pattern="include/diagnostics/*",
            src=artifacts,
            dst=self.package_folder,
        )
        copy(self, pattern="lib/*", src=artifacts, dst=self.package_folder)
        copy(self, pattern="protobuf/*", src=artifacts, dst=self.package_folder)
        os.makedirs(path.join(self.package_folder, "protobuf-proto"))
        with open(
            path.join(self.package_folder, "protobuf-proto/README.md"), "w"
        ) as proto_warning:
            proto_warning.write(
                textwrap.dedent(
                    """\
                # **DO NOT REGENERATE** .pb.cc AND .pb.h FILES USING THESE!
                These files are for reference only.

                Serialized protobuf is backwards and forwards compatible, but protobuf library structures **are not.**
                Regenerating using these could:
                * cause your application to suddenly not link in the future
                * cause your application and the library to pass differently packed structures to each other, with each assuming the other has the same structure definition
            """
                )
            )
        copy(
            self,
            pattern="*.proto",
            src=path.join(self.folders.source_folder, "external/middleware-proto"),
            dst=path.join(self.package_folder, "protobuf-proto"),
        )
        copy(
            self, pattern="apps/DiscoveryServer", src=artifacts, dst=self.package_folder
        )
        copy(self, pattern="apps/ShmCleanup", src=artifacts, dst=self.package_folder)

    def deploy(self):
        copy(self, pattern="lib/*.so", src=self.package_folder, dst=self.deploy_folder)

    def package_info(self):
        self.cpp_info.libs = [
            "MiddlewareParameterService",
            "MiddlewareDiagnosticsService",
        ]
        self.cpp_info.includedirs = [
            "include/protobuf",
            "include/framework",
            "include/parameters",
            "include/diagnostics",
        ]

        # Dependency targets + their needs
        fast_dds_requires = ["fast-dds::fast-dds", "tinyxml2::tinyxml2", "asio::asio", "fast-cdr::fast-cdr", "foonathan-memory::foonathan-memory"]
        base_requires = ["protobuf::libprotobuf", "abseil::abseil", "boost::boost", "bzip2::bzip2", "pugixml::pugixml"] + fast_dds_requires
        # package's cmake properties
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_target_name", "cate_middleware::cate_middleware")
        self.cpp_info.set_property("cmake_file_name", "cate_middleware")
        # For Parameter API
        self.cpp_info.components["ParameterService"].libs = [
            "MiddlewareParameterService"
        ]
        self.cpp_info.components["ParameterService"].includedirs = [
            "include/protobuf",
            "include/parameters",
            "include/framework",
        ]
        self.cpp_info.components["ParameterService"].requires = base_requires
        # For Diagnostics API
        self.cpp_info.components["DiagnosticsService"].libs = [
            "MiddlewareDiagnosticsService"
        ]
        self.cpp_info.components["DiagnosticsService"].includedirs = [
            "include/protobuf",
            "include/diagnostics",
            "include/framework",
        ]
        self.cpp_info.components["DiagnosticsService"].requires = base_requires
        # Header only for client test needs
        self.cpp_info.components["ServiceHeaders"].includedirs = [
            "include/protobuf",
            "include/parameters",
            "include/diagnostics",
        ]
        self.cpp_info.components["ServiceHeaders"].requires = []
        self.cpp_info.components["ServiceHeaders"].libs = []

        # conan is not picking up our cpp_info.libdirs even if explicitly set and with us giving components info
        # could be a generators issue, not package_info, according to the docs
        # doing this manually makes runenvs work, but puts the user's old variables in the middle of the new ones
        # conan currently (erroneously) overwrites instead of appending to the user's when setting up a runenv
        # once they fix that this will cause their old path to be repeated 2x
        # however, define_path makes no other packages' lib paths make it in at all
        self.runenv_info.append_path(
            "LD_LIBRARY_PATH", path.join(self.package_folder, "lib")
        )
        self.runenv_info.append_path(
            "DYLD_LIBRARY_PATH", path.join(self.package_folder, "lib")
        )
