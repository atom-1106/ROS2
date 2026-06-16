# Release Gen 7 Middleware Software <!-- omit in toc -->

- [Definitions](#definitions)
- [Overview](#overview)
- [Artifactory Remotes](#artifactory-remotes)
- [Release Dependencies](#release-dependencies)
- [Package Release Locations](#package-release-locations)
- [Package Versions](#package-versions)
- [Release Package Creation and Upload](#release-package-creation-and-upload)
- [Under Review: Package Revisions](#under-review-package-revisions)
- [Release Notes](#release-notes)
- [Project Tagging](#project-tagging)
- [Email Notification](#email-notification)


## Definitions
| Name               | Definition                                                                                                             |
| ------------------ | ---------------------------------------------------------------------------------------------------------------------- |
| Root               | The root of the superproject where the directory would be called `middleware-cpp`.                                     |
| Conan              | Package Manager System Tool we use to build, containerize, and distribute software.                                    |
| Package            | Conan generated container that contains built binaries, metadata, and any other releasable content.                    |
| Build              | Conan commands that target source CMake locally in the repository.                                                     |
| Create             | Conan commands that target source CMake exported into the machine's conan cache.                                       |
| Upload             | Conan commands that upload a built package and it's revisions to an authorized and configured remote.                  |
| Package Promotion  | The process of copying a package version by revision ID to a production space then deleting it from development space. |
| Open Source Remote | CAT Artifactory remote for open source Conan Packages labeled for production usage.                                    |
| Production Remote  | CAT Artifactory remote for CAT Software Conan Packages labeled for production usage.                                   |
| Development Remote | CAT Artifactory remote for CAT Software Conan Packages pending promotion to Production Remote.                         |

## Overview
This document will detail the process/processes followed for releasing Middleware Conan Software Packages for consumption by clients and users.
For in-depth details related to Gen 7 release process see [Bogtrotter Releasing](https://gitgis.ecorp.cat.com/es-csf-core-info/gen7/bogtrotter_docs/-/blob/master/releasing.md?ref_type=heads).
The process follows these steps in order:

1. Determine tagging/version values.
2. Initial Package Creation and Upload.
3. Package becomes Under Review and is downloaded from remote and tested.
   1. Any issues or bugs are fixed.
   2. Fixes result in a revision upload.
4. Remote Administrator is notified of the package by name, version, and revision ID for promotion.
5. Release notes CHANGELOG.md is updated.
6. Source repository is tagged.
7. Users and consumers are notified by email.

## Artifactory Remotes
Currently, the following remotes are available for usage but are subject to change over time:

- Open Source Remote
  - Current: `https://stuff.ecorp.cat.com/artifactory/api/conan/cat-e-conan-center-dev-local`
- Production Remote
  - Current: `https://stuff.ecorp.cat.com/artifactory/api/conan/cat-e-conan-rel-local`
- Development Remote
  - Current: `https://stuff.ecorp.cat.com/artifactory/api/conan/cat-e-conan-dev-local`

## Release Dependencies
The Middleware Release Package is required to consume its dependencies from two locations:
- Open Source Remote
- Production Remote

Remote `https://center2.conan.io` is not a remote for CAT Production Software to download dependencies from for software needs. 
If content is not in **Open Source Remote** or **Production Remote**, please contact the CAT Artifactory Administrators.


## Package Release Locations
The Middleware Release Package is uploaded to the **Development Remote** for Remote Administrators to evaluate.
Only the last uploaded package revision will be evaluated.
Once the package passes evaluation, the Remote Administrator will promote the package to the **Production Remote**, which is the remote consumers will register to for content.
The Remote Administrator will delete the Middleware Release Package and all of its revisions from **Development Remote** once the move is complete.


## Package Versions
The Middleware Release Package hardcodes a package version value into its recipe.
This version value will follow semantical versioning format `major.minor.patch`.


## Release Package Creation and Upload
A release package will be created using a source repository from the master branch with a passing pipeline build.
Before executing conan creation commands, the entire conan cache on the machine creating the package will be cleaned.
The steps are:
```bash
# [Required] Create the Middleware conan package binaries
./create.sh 1.0.0
# Upload the Middleware conan package
./upload.sh 1.0.0
```

## Under Review: Package Revisions
The Middleware Release Package is **Under Review** when the package is currently available in **Development Remote** but has not been moved to **Production Remote**.
Packages under review can be uploaded as many times as needed, which in conan they are called **Package Revisions**.
The revisions are made due to bugs or issues discovered while testing the release package.


## Release Notes
Release Notes are captured via the CHANGELOG.md in the root of the repository.
The latest notes will be appended to the top.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
The updates will be made before Git tagging the repository.


## Project Tagging
Once the Middleware Release Package has been promoted, the source repository will be tagged in the same format as package version except the project's name will be appended to the front.
This would look like `Middleware_major.minor.patch`.
When tagging a `middleware-cpp` repository, **just middleware-cpp** is to be tagged, do not tag any submodules. 


## Email Notification
Once the Middleware Release Package is available in the promotion remote and the project repository has been tagged, the release engineer is responsible for sending an email to all parties and consumers that use or require Middleware.
The email will contain the following information:
- Package Name
- Package Version
- URL to the source repository by Git Tag
- URL links to the binary packages in Artifactory by revision IDs
- Short list of notable changes
