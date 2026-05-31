# AtGames General Loader (GL) 2.0 SDK: Developer Quick Start Guide

## Overview

The General Loader 2.0 SDK enables third-party developers to build, compile, and deploy native C++ SDL2 applications for AtGames arcade hardware.

**Security & Storage Policy (The Sandbox):**
To protect system integrity, all GL 2.0 applications run within a strict sandbox.

* **Local Storage:** Applications only have read/write access to their own specific application directory on the mounted USB drive. They cannot access the arcade's internal storage or other USB partitions.
* **Cloud Storage:** Developers wishing to implement global leaderboards or cross-device saves must provide and manage their own cloud API endpoints. AtGames does not host third-party application data.

---

## Phase 1: Environment Setup

The SDK utilizes a Dockerized Ubuntu 16.04 cross-compilation toolchain (`arm-a53-toolchain`) to ensure your application is perfectly compatible with the arcade firmware.

1. **Install Docker** on your Windows, Mac, or Linux development machine.
2. **Launch the Toolchain:** Use the provided launcher scripts in your root directory to build and enter the compiler environment.

**For Windows (`run.bat`):**

```bat
@echo off
REM Check if the Docker image already exists
docker image inspect arm-a53-toolchain:latest >nul 2>&1
if not %errorlevel%==0 (
  docker build -t arm-a53-toolchain:latest .
)
if not "%~1"=="" if exist "media\%~1" (
  docker run -it -v "%cd%\media:/workspace" arm-a53-toolchain:latest /bin/bash -c "./build.sh \"%~1\""
) else (
  docker run -it -v "%cd%\media:/workspace" arm-a53-toolchain:latest /bin/bash
)

```

**For Linux/Mac (`run.sh`):**

```bash
#!/bin/bash +x
if ! docker image ls | grep -q "arm-a53-toolchain" ; then
  docker build -t arm-a53-toolchain:latest .
fi
if [ "$1" ] && [ -d "media/$1" ]; then
    docker run -it -v "$(pwd)/media:/workspace" arm-a53-toolchain:latest /bin/bash -c "./build.sh \"$1\""
else
    docker run -it -v "$(pwd)/media:/workspace" arm-a53-toolchain:latest /bin/bash
fi

```

---

## Phase 2: Project Architecture & Source Code

For a complete, working example of an SDL2 game loop, input handling, and rendering, please refer to the official sample repository:
**[GitHub: Sample Puzzle Game](https://github.com/thelastknightahk/sample-puzzle-game)**

### Workspace Structure

Your development `media/` folder must contain both your project folder AND the `toolchain` directory for the compiler to work correctly. A standard project requires the following layout:

```text
media/
├── puzzle-example/
│   ├── src/
│   │   ├── main.cpp
│   │   └── PuzzleLogic.cpp
│   ├── CMakeLists.txt
│   └── USB/
│       ├── puzzle-example.xml
│       └── puzzle-example.png
└── toolchain/
    └── a53-toolchain.cmake

```

### Build Configuration (`CMakeLists.txt`)

Your project must include a `CMakeLists.txt` file linked against the necessary SDL2 libraries available in the firmware environment:

```cmake
cmake_minimum_required(VERSION 3.0)
project(puzzle-example)

set(CMAKE_CXX_STANDARD 11)

# Add source files
add_executable(puzzle-example src/main.cpp src/PuzzleLogic.cpp)

# Link required AtGames firmware graphics/audio libraries
target_link_libraries(puzzle-example SDL2 SDL2_image SDL2_ttf SDL2_mixer)

```

---

## Phase 3: The Application Manifest

For the arcade menu to recognize and launch your application, you must provide an XML manifest and an icon. Place these in your project's `USB/` directory.

**1. Icon (`puzzle-example.png`):** A square PNG image used for the menu thumbnail.
**2. Manifest (`puzzle-example.xml`):** Defines the application metadata.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<app>
    <name>Puzzle Prime</name>
    <executable>puzzle-example.elf</executable>
    <icon>puzzle-example.png</icon>
    <version>2.0</version>
</app>

```

*Note: The `<executable>` tag must exactly match the output name of your compiled `.elf` binary.*

---

## Phase 4: Compiling the Binary (.elf)

Open your terminal and mount your local `media` folder directly to `/workspace` inside the container:

```bash
# Example Mount Command (Adjust path to your local media folder)
docker run -it -v "C:\path\to\your\test_game\media:/workspace" arm-a53-toolchain /bin/bash

```

Once inside the container (`root@...:/workspace#`), execute this exact sequence to compile the code and export the `.elf` file:

```bash
# 1. Prepare a clean workspace to ensure high performance and correct permissions
rm -rf /tmp/puzzle-example
cp -r /workspace/puzzle-example /tmp/puzzle-example
cd /tmp/puzzle-example

# 2. Initialize a fresh build environment
rm -rf build
mkdir build
cd build

# 3. Configure the project using the cross-compilation toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=/workspace/toolchain/a53-toolchain.cmake

# 4. Compile the source code into the executable
cmake --build .

# 5. Export the final ELF file back to your host machine's USB folder
cp puzzle-example /workspace/puzzle-example/USB/puzzle-example.elf

```

---

## Phase 5: Arcade Deployment

The AtGames firmware expects a strict folder hierarchy on external storage.

1. Format a USB Flash Drive to **FAT32**.
2. Create a folder named `external` at the root of the drive.
3. Copy your compiled project folder (containing the `.elf`, `.png`, and `.xml`) inside the `external` folder.

**Final USB Structure:**

```text
USB_DRIVE (FAT32)/
└── external/
    └── puzzle-example/
        ├── puzzle-example.elf
        ├── puzzle-example.png
        └── puzzle-example.xml

```

Insert the USB drive into your AtGames arcade machine. Navigate to the **AppStoreX / External Applications** tab to view and launch your application.
 