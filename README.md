# cxxg
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](./LICENSE) ![GitHub CI](https://img.shields.io/github/actions/workflow/status/Manewing/cxxg/.github%2Fworkflows%2Fcmake-multi-platform.yml)

A set of C++ console games for Unix systems together with a small library for accessing and modifiying the terminal screen.

List of games:
- 2048:
- MasterMind:
- Tetris

# Build
In order to build `cxxg` you need `cmake (version >= 3.5)`.

1. Initialize submodules
    ```
    git submodule update --init --recursive
    ```

2. Install `cmake`:
    ```
    sudo apt install cmake # Ubuntu
    brew install cmake # Mac OS (brew)
    ```

3. Configure and build `cxxg`, default `BUILD_TESTS=OFF`:
    ```
    cd cxxg;
    mkdir build && cd build;
    cmake ../ -DBUILD_TESTS=[ON/OFF]
    make
    ```

# Screenshots
## 2048
![Screenshot 2048](doc/screenshots/2048.png)
## MasterMind
![Screenshot MasterMind](doc/screenshots/mastermind.png)
