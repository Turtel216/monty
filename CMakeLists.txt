cmake_minimum_required(VERSION 3.30)
project(lang LANGUAGES CXX)

# Enable testing
include(CTest)

# Source files
set(SRC_FILES
    src/main.cpp
    src/lexer.cpp
)

# Main executable
add_executable(ggc ${SRC_FILES})

# Tests
#if (BUILD_TESTING)
#    add_executable(lexer_test_success tests/lexer/test_success.cpp)
#    target_link_libraries(lexer_test_success lexer_lib)
#endif()

# C++ standard
set_target_properties(ggc PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

