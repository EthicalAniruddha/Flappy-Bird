cmake_minimum_required(VERSION 3.10)
project(FlappyBird)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_STANDARD 11)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(app src/main.c)

target_link_libraries(app PRIVATE raylib m pthread dl)
target_include_directories(app PRIVATE /usr/local/include)

add_custom_target(run
    COMMAND app
    DEPENDS app
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)