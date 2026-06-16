# Agent Setup Instructions

This file contains required environment setup steps that must be followed before building or running in this repository.

## Required build environment setup (Example: tgt_gcc_x86_64_ubuntu_workstation Debug)

Before any build, test, or run that depends on the Debug toolchain for the x86_64 Ubuntu workstation target, you **must** source the Conan generator environment scripts:

```bash
source build/tgt_gcc_x86_64_ubuntu_workstation/Debug/generators/conanbuild.sh
source build/tgt_gcc_x86_64_ubuntu_workstation/Debug/generators/conanrun.sh
```

These scripts set required compiler, linker, and runtime environment variables. Skipping this step will lead to build or runtime failures.

## Notes

- If you switch build type or target, use the corresponding `build/<target>/<config>/generators` directory and source both scripts from there.
- Run these in the same shell session as your build/test commands.
- If a build fails due to missing open source dependencies or missing generator scripts, run Conan install via the build utility:
	- Example: `./build-utilities/build.sh dev arm64 relwithdebinfo`
	- This regenerates the required Conan generators and dependencies for the target/config.
