# Game Template

This is a simple repository template I use for starting new game projects. It has the basic outline of a project structure
already setup as well as a few systems implemented, such as a basic platform layer, renderer, asset management system, etc.

Currently the system supports building for Windows and Web Assembly. It can also build to MacOS but no work has
been done to *officially* support the platform, so the generated .app bundles are missing things like an info plist.
The back-end of the engine uses SDL2 so it should be easy to also port to other systems e.g. Linux, Android, iOS, etc.

## Building

After cloning the repository you will need to run `git submodule update --init --recursive` to setup submodules.

The project can then be built by running either the `build\win32\build.bat` or `build\macos\build.sh` script with a build
profile as the first argument. The following build profiles are available:

- `win32` which builds the Windows version of the project.
- `macos` which builds the MacOS version of the project.
- `web` which builds the Web Assembly version of the project.
- `tools` which builds auxiliary tools used for development.

You can also specify an extra `release` argument to build an optimized executable with debug information stripped.

## License

The project's code is available under the **[MIT License](https://github.com/JROB774/game-template/blob/master/LICENSE)**.
