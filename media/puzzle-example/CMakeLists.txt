cmake_minimum_required(VERSION 3.0)
project(puzzle-example)

set(CMAKE_CXX_STANDARD 11)

# Add source files
add_executable(puzzle-example src/main.cpp src/PuzzleLogic.cpp)

# Link required AtGames firmware graphics/audio libraries
target_link_libraries(puzzle-example SDL2 SDL2_image SDL2_ttf SDL2_mixer)