cmake_minimum_required(VERSION 3.14)

project(example_pipeline_plugin VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)

# Find required packages
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core RIO)
find_package(AnalysisPipelineStages REQUIRED)

# Include ROOT macros like ROOT_GENERATE_DICTIONARY
include(${ROOT_USE_FILE})

# Get include directories from AnalysisPipelineStages imported target
get_target_property(APS_INCLUDE_DIRS AnalysisPipelineStages::analysis_pipeline_stages INTERFACE_INCLUDE_DIRECTORIES)

# Gather source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Set include directories for your target
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${APS_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        AnalysisPipelineStages::analysis_pipeline_stages
        ROOT::Core ROOT::RIO
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)

# ROOT dictionary generation
set(ROOT_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/LinkDef.h)

# Prepare include directories for ROOT dictionary generation (following unpacker pattern)
set(ALL_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${APS_INCLUDE_DIRS}
)
list(REMOVE_DUPLICATES ALL_INCLUDE_DIRS)

# Prepare ROOT dictionary include options for generation
set(ROOT_DICTIONARY_INCLUDE_OPTIONS "")
foreach(dir IN LISTS ALL_INCLUDE_DIRS)
  list(APPEND ROOT_DICTIONARY_INCLUDE_OPTIONS "-I${dir}")
endforeach()

# Generate dictionary using lowercase root_generate_dictionary (like unpacker)
root_generate_dictionary(
    G__${PROJECT_NAME}
    "stages/examples/constant_value_stage.h"
    MODULE ${PROJECT_NAME}
    LINKDEF ${ROOT_LINKDEF}
    OPTIONS ${ROOT_DICTIONARY_INCLUDE_OPTIONS}
)

# Add generated dictionary source
target_link_libraries(${PROJECT_NAME} PRIVATE G__${PROJECT_NAME})

# Include dictionary generated headers for your target
target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}  # for generated dictionary headers
)

# Installation rules
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Package config generation and install
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
