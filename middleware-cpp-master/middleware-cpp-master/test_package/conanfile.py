# https://gitgis.ecorp.cat.com/es-csf-core-info/gen7/bogtrotter_docs/-/blob/master/bogtrotters/releases/middleware_instructions.md

import conan
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.env import VirtualRunEnv
from conan.tools.build import can_run
from os import path

class TestConanProject(conan.ConanFile):
    name = "cate_middleware-test"
    version = "0.1.0"
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch", "cat_tgt"
    exports_sources = "CMakeLists.txt", "src/*"
    generators = "CMakeDeps", "CMakeToolchain", "VirtualRunEnv"

    def requirements(self):
        self.requires(self.tested_reference_str)

    def layout(self):
        cmake_layout(self, build_folder=path.join("build", self.settings.cat_tgt.value, self.settings.build_type.value))

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        if can_run(self):
            headers_exe = path.join(self.build_folder, "MiddlewareTestPackage_headers")
            parameters_exe = path.join(self.build_folder, "MiddlewareTestPackage_parameters")
            diagnostics_exe = path.join(self.build_folder, "MiddlewareTestPackage_diagnostics")
            primary_exe = path.join(self.build_folder, "MiddlewareTestPackage_primary")
            
            self.run(headers_exe, env="conanrun")
            self.run(parameters_exe, env="conanrun")
            self.run(diagnostics_exe, env="conanrun")
            self.run(primary_exe, env="conanrun")

