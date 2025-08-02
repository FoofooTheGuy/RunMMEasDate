cmake_minimum_required(VERSION 3.10)

# Project name and version
project(RunMMEasDate VERSION 1.0 LANGUAGES CXX)

# Disable just_my_code_debugging
set (CMAKE_VS_JUST_MY_CODE_DEBUGGING 0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")

# Define the source files for the DLL
set(DLL_SOURCES
    DateInject.cpp
)

# Path to MinHook library (using relative path)
set(MINHOOK_DIR "${CMAKE_SOURCE_DIR}/minhook")
set(MINHOOK_INCLUDE_DIR "${MINHOOK_DIR}/include")
# Add MinHook as a subdirectory to build it as part of the project
add_subdirectory(${MINHOOK_DIR} minhook_build)

# MinHook library will be available as 'minhook' target after add_subdirectory
set(MINHOOK_LIB minhook)

# Create the DLL (shared library)
add_library(DateInject SHARED ${DLL_SOURCES})

# Set output name for the DLL
set_target_properties(DateInject PROPERTIES
    OUTPUT_NAME "DateInject"
    PREFIX ""
    SUFFIX ".dll"
)

# Include directories for the DLL
target_include_directories(DateInject PRIVATE ${MINHOOK_INCLUDE_DIR})

# Link the DLL to MinHook library
target_link_libraries(DateInject PRIVATE ${MINHOOK_LIB})

# Define the source files for the executable
set(EXE_SOURCES
    main.cpp
)

# Create the executable
add_executable(RunMMEasDate ${EXE_SOURCES})

# Link the executable to required libraries if needed
# Currently, the main.cpp does not directly depend on MinHook, so no linking is necessary here
# If additional dependencies are needed, they can be added with target_link_libraries

# Set output directories for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Ensure the DLL is built before the executable
add_dependencies(RunMMEasDate DateInject)
