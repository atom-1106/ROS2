# Gen 7 Middleware Topic Generation <!-- omit in toc -->

- [Introduction](#introduction)
- [Requirements](#requirements)
- [Generation](#generation)


## Introduction
Middleware using eProsima Fast DDS which requires the usage of Fast DDS Topics.
Topics are IDL files that are generated in C++ code that houses data types, functions, classes, and any functionality that is used to communicate between to network endpoints.
This document details how to generate the C++ code of the Topic IDL files.

## Requirements
Generation of code is done via [eProsima's Fast DDS Gen](https://github.com/eProsima/Fast-DDS-Gen) application.
That application is provided as a Conan Package Build Tool.
The [regenerate.sh](regenerate.sh) script will call conan commands that target the generation recipe.

## Generation
Run the `regenerate.sh` script, it takes no arguments.

```bash
./regenerate.sh
```

The generated code will be placed in the subdirectory that matches the IDL name.
For example, `message.idl` is located [here](message/message.idl).
So, the generated code will be placed in the same directory.

**IMPORTANT**
- Generated code is not clang formatted. The code will need to be formatted with clang before pushing changes.
- Generated code is not analyzed by `cppcheck` or `flawfinder`.

