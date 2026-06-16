# Gen 7 Middleware <!-- omit in toc -->

- [Introduction](#introduction)
- [Supported Platforms](#supported-platforms)
- [Project Layout](#project-layout)
  - [build-utilities](#build-utilities)
  - [scripts](#scripts)
  - [doc](#doc)
  - [external](#external)
  - [lib](#lib)
  - [apps](#apps)
- [Software Guide](#software-guide)
- [Software Building](#software-building)
- [Fast DDS IDL Code Generation](#fast-dds-idl-code-generation)
- [Middleware Releases](#middleware-releases)

## Introduction
Middleware is a collection of configurable shared libraries that handle data transfers between onboard and offboard users via a provided API and internal framework. The Middleware follows the "publisher/subscriber" model of transfer which is asynchronous point-to-point communication and not a centralized server. The data packet transferred is topic structures. Topics contain the core data expected to be gathered on each endpoint. Users of the Middleware cannot see topics or the internal framework, their line of sight stops at the APIs.

## Supported Platforms
- **tgt_gcc_arm64_guest**: This platform is used for A7 ARM64 hardware.
- **tgt_gcc_x86_64_ubuntu_workstation**: This platform is used to test software on local Linux PCs, i.e Google Unit Tests or during development.

## Project Layout

### build-utilities
This is a directory that contains build scripts and their dependencies to build each platform target. See [further documentation](https://gitgis.ecorp.cat.com/es-csf-core-info/gen7/bogtrotter_docs/-/blob/master/bogtrotters/releases/middleware_conan_releases.md) on this section.

### scripts
This is a directory that contains scripts and tools unrelated to building software. These can be scripts and tools for releasing software to clients, GitLab helpers, file creation helpers, and project tagging helpers.

### doc
This is a directory that contains documentations that contain information related to:

- Middleware APIs
- Middleware Designs
- Middleware Usage
- Diagrams
- Images

### external
This is a directory that contains Git repositories that are external dependencies not managed by conan. They are submodules to "middleware-cpp". It also contains its own CMakeLists.txt that packages those repositories into interface libraries to be used by Middleware.

### lib
This is a directory that contains all Middleware's source code.

### apps
This is a directory that contains small project applications used to test, debug, and gather resource usage data from the Middleware libraries.

## [Software Guide](doc/software-guide.md)

## [Software Building](doc/software-building.md)

## [Fast DDS IDL Code Generation](lib/framework/dds/topics/README.md)

## Middleware Releases
Middleware is released as a conan package to CAT Artifactory.
The Middleware team is also responsible for distributing its core dependency Fast DDS, which is released in the same way as its own content.
For information on this process, see the following:

- [Middleware Release Process](doc/software-releasing.md)
- [Conan Release Process(deprecated)](https://gitgis.ecorp.cat.com/es-csf-core-info/gen7/bogtrotter_docs/-/blob/master/bogtrotters/releases/middleware_conan_releases.md)

