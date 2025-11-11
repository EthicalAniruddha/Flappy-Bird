cmake_minimum_required(VERSION 3.10)
project(FlappyBird)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_STANDARD 11)

# Copy assets to output directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(app src/main.c)

# Platform-specific linking
if(WIN32)
    # Windows: Use the Windows-built Raylib
    target_include_directories(app PRIVATE $ENV{HOME}/raylib/src)
    
    target_link_libraries(app PRIVATE 
        $ENV{HOME}/raylib/src/libraylib.a
        opengl32 gdi32 winmm
    )
    
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc")
else()
    # Linux: Your original setup
    target_link_libraries(app PRIVATE raylib m pthread dl)
    target_include_directories(app PRIVATE /usr/local/include)
endif()

# Run target (works on Linux)
add_custom_target(run
    COMMAND app
    DEPENDS app
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)