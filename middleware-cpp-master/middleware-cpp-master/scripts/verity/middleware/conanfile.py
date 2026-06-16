from conan.tools.files import copy
import os
from os import path
import re
from shlex import quote
import createverity

class Verity(createverity.CreateVerity):
    name = "middleware-verity"
    version = "2025.3.0"
    requires = ["cate_middleware/" + version]

    settings = "os", "build_type", "arch"
    keep_imports = True
    STAGING_DIR = "staging"
    OUTPUT_DIR = "verity"
    @property
    def VERITY_NAME(self):
        name = "Middleware_ARM64_GUEST_"
        return name + re.sub(r'[^0-9a-zA-Z.-]', "_", self.version)

    def generate(self):
        toolchain_search = dict((k, v) for k, v in self.dependencies.build.items() if v.buildenv_info.vars(self).get("STRIP") is not None)
        if(len(toolchain_search) == 0):
           raise Exception("Failed to find executable to strip with!")
        if(len(toolchain_search) > 1):
           raise Exception("Found multiple executables we could strip with ({})!".format(", ".join(k.ref for k in toolchain_search.keys())))
        strip = list(toolchain_search.values())[0].buildenv_info.vars(self).get("STRIP")

        transitive_deps = set()
        for dependency, package in self.dependencies.items():
            transitive_deps.update(k.ref for k, v in package.dependencies.items())
        for dependency, package in self.dependencies.items():
            if not dependency.build:
                if package.cpp_info.libdirs:
                    copy(self,
                         pattern="*.so*",
                         dst=path.join(self.folders.build_folder, self.STAGING_DIR, "lib"),
                         src=package.cpp_info.libdirs[0])
                if package.cpp_info.bindirs and dependency.ref not in transitive_deps:
                    bindir = path.join(self.folders.build_folder, self.STAGING_DIR, "bin")
                    copy(self,
                         pattern="*",
                         dst=bindir,
                         src=package.cpp_info.bindirs[0])
                    for root, _, files in os.walk(bindir):
                        for file in files:
                            if not os.access(path.join(root, file), os.X_OK):
                                raise Exception("Non-binary file found in bindir: " + file)

        for root, _, files in os.walk(path.join(self.folders.build_folder, self.STAGING_DIR, "lib")):
            for file in files:
                self.run(strip + " " + quote(path.join(root, file)))
        for root, _, files in os.walk(path.join(self.folders.build_folder, self.STAGING_DIR, "bin")):
            for file in files:
                self.run(strip + " " + quote(path.join(root, file)))

    def build(self):
        super().create_verity(self.VERITY_NAME)
