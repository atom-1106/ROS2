# Build Gen 7 Middleware Software <!-- omit in toc -->

- [Definitions](#definitions)
- [Setup: Docker](#setup-docker)
- [Build](#build)
- [Create](#create)
- [Deployable Artifacts](#deployable-artifacts)
- [Executable Tests](#executable-tests)


## Definitions
| Name    | Definition                                                                                          |
| ------- | --------------------------------------------------------------------------------------------------- |
| Root    | The root of the superproject where the directory would be called `middleware-cpp`.                  |
| Conan   | Package Manager System Tool we use to build, containerize, and distribute software.                 |
| Package | Conan generated container that contains built binaries, metadata, and any other releasable content. |
| Build   | Conan commands that target source CMake locally in the repository.                                  |
| Create  | Conan commands that target source CMake exported into the machine's conan cache.                    |

## Setup: Docker
The software is expected to be built in a container from a defined image. See [Development Tools Repository](https://gitgis.ecorp.cat.com/es-csf-core-info/info-services/development-tools/-/tree/master/docker?ref_type=heads)

## Build
Middleware uses a build script to run the `conan build` command to build and package Middleware content locally from the repository.
It will build a package using the configurations `Debug` and `RelWithDebInfo` for the CMake build targets local to the repository.

**Build Steps**

1) cd {ROOT}/build-utilities
2) ./build {version tag} {additional args}

```bash
# Build all
./build.sh 1.0.0
# Build x86 all configurations
./build.sh 1.0.0 x86
# Build x86 just Debug
./build.sh 1.0.0 x86 debug
# Build x86 just RelWithDebInfo
./build.sh 1.0.0 x86 relwithdebinfo
# Build arm64 all configurations
./build.sh 1.0.0 arm64
# Build arm64 just Debug
./build.sh 1.0.0 arm64 debug
# Build arm64 just RelWithDebInfo
./build.sh 1.0.0 arm64 relwithdebinfo 
```

## Create
Middleware uses a build script to run the `conan create` command to build and package Middleware content from inside `conan cache`.
Conan cache is the only package build that can be uploaded to remotes.
It will create a package using the configurations `Debug` and `RelWithDebInfo` for the CMake build targets exported to `conan cache`.
The created conan package is saved to `artifacts/{platform}/{build configuration}/package/conan_cache_save.tgz` to be restored later for usage.

**!!IMPORTANT!!** \
The `create.sh` can not be ran if the Git history is dirty. All changes in this repository must be committed or cleaned before running the script.

**Create Steps**

1) cd {ROOT}/build-utilities
2) ./create {version tag} {additional args}

```bash
# Create all
./create.sh 1.0.0
# Create x86 all configurations
./create.sh 1.0.0 x86
# Create x86 just Debug
./create.sh 1.0.0 x86 debug
# Create x86 just RelWithDebInfo
./create.sh 1.0.0 x86 relwithdebinfo
# Create arm64 all configurations
./create.sh 1.0.0 arm64
# Create arm64 just Debug
./create.sh 1.0.0 arm64 debug
# Create arm64 just RelWithDebInfo
./create.sh 1.0.0 arm64 relwithdebinfo 
```

## Deployable Artifacts
Once Middleware is built, there will be a directory called "{root}/artifacts" that is created. Artifacts contains all the unstriped built binaries. Artifacts layout can vary from platform to platform. See the layout below:

- {ROOT}/artifacts/tgt_gcc_arm64_guest
    - Debug: Contains deployable software built in Debug configuration.
        - apps: Built executable applications like tools and checkers for Middleware
        - conan: Dependency binaries built by conan
        - include: Required header files for clients to integrate Middleware
        - lib: Contains the built project libraries
        - protobuf: Generated header and source files from Protobuf files
        - package: Saved conan package and its informational json file
    - RelWithDebInfo: Contains deployable software built in RelWithDebInfo configuration.
        - apps: Built executable applications like tools and checkers for Middleware
        - conan: Dependency binaries built by conan
        - include: Required header files for clients to integrate Middleware
        - lib: Contains the built project libraries
        - protobuf: Generated header and source files from Protobuf files
        - package: Saved conan package and its informational json file
- {ROOT}/artifacts/tgt_gcc_x86_64_ubuntu_workstation
    - Debug: Contains deployable software built in Debug configuration.
        - apps: Built executable applications like tools and checkers for Middleware
        - conan: Dependency binaries built by conan
        - include: Required header files for clients to integrate Middleware
        - lib: Contains the built project libraries
        - protobuf: Generated header and source files from Protobuf files
        - package: Saved conan package and its informational json file
        - tests: Built Google Test applications
    - RelWithDebInfo
        - Contains deployable software built in RelWithDebInfo configuration.
        - apps: Built executable applications like tools and checkers for Middleware
        - conan: Dependency binaries built by conan
        - include: Required header files for clients to integrate Middleware
        - lib: Contains the built project libraries
        - protobuf: Generated header and source files from Protobuf files
        - package: Saved conan package and its informational json file
        - tests: Built Google Test applications


## Executable Tests
When building the tgt_gcc_x86_64_ubuntu_workstation platform, Google Test binaries will be created in artifacts. Running these tests requires them to be ran in the artifacts directory it resides or the dependencies in `artifacts/{platform}/{build configuration}/conan` need exported before execution.

**Inside artifacts:**

1) cd {root}/artifacts/tgt_gcc_x86_64_ubuntu_workstation/{Debug,RelWithDebInfo}/tests
2) ./MiddlewareTest
3) ./IntegrationTest

**Outside artifacts:**

1) export LD_LIBRARY_PATH={root}/artifacts/tgt_gcc_x86_64_ubuntu_workstation/{Debug,RelWithDebInfo}/conan
2) ./{root}/artifacts/tgt_gcc_x86_64_ubuntu_workstation/{Debug,RelWithDebInfo}/tests/MiddlewareTest
3) ./{root}/artifacts/tgt_gcc_x86_64_ubuntu_workstation/{Debug,RelWithDebInfo}/tests/IntegrationTest
