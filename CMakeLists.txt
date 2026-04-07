cmake_minimum_required(VERSION 3.10)
project(JRON C)

# Use C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Put compiled binaries in a predictable location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Public include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all C sources from src/
file(GLOB_RECURSE JRON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.c
)

# Exclude the application entrypoint from the library sources (avoid duplicate main)
list(REMOVE_ITEM JRON_SOURCES ${CMAKE_SOURCE_DIR}/src/main.c)

# Create a static library from sources (so other targets can link)
add_library(jron STATIC ${JRON_SOURCES}
        include/topic.h
        include/topic.h
        include/jron.h
        include/broker.h
        include/generic_dictionary.h
        include/topic_dictionary.h
        src/topic.c
        include/jron_protocol.h
        src/socket.c
        include/socket.h
)

target_include_directories(jron PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Example executable
add_executable(jron_app src/main.c)

target_link_libraries(jron_app PRIVATE jron)

# Compiler warnings for common compilers
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(jron PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(jron_app PRIVATE -Wall -Wextra -pedantic)
elseif(MSVC)
    target_compile_options(jron PRIVATE /W4)
    target_compile_options(jron_app PRIVATE /W4)
endif()

# Installation (optional)
install(TARGETS jron_app RUNTIME DESTINATION bin)
install(TARGETS jron ARCHIVE DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

# Helpful message when configured
message(STATUS "Configured JRON project")
