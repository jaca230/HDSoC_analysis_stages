cmake_minimum_required(VERSION 3.14)

project(hdsoc_analysis_stages VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Suppress false-positive GCC warnings when top-level
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-stringop-overflow)
  endif()
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ------------------ CPM Setup ------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

# spdlog (header-only)
CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.13.0
  OPTIONS "SPDLOG_BUILD_EXAMPLES OFF" "SPDLOG_BUILD_TESTS OFF" "SPDLOG_FMT_EXTERNAL OFF"
  DOWNLOAD_ONLY YES
)

if(spdlog_ADDED)
  add_library(spdlog_header_only INTERFACE)
  target_include_directories(spdlog_header_only INTERFACE
    $<BUILD_INTERFACE:${spdlog_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  add_library(spdlog::spdlog ALIAS spdlog_header_only)
endif()

# nlohmann_json (header-only)
CPMAddPackage(
  NAME nlohmann_json
  GITHUB_REPOSITORY nlohmann/json
  VERSION 3.11.3
  DOWNLOAD_ONLY YES
)

if(nlohmann_json_ADDED)
  add_library(nlohmann_json_header_only INTERFACE)
  target_include_directories(nlohmann_json_header_only INTERFACE
    $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json_header_only)
endif()

# Dependencies
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
include(${ROOT_USE_FILE})
find_package(AnalysisPipelineStagesCore REQUIRED)
find_package(unpacker REQUIRED)

# Sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx
)

# Headers for ROOT dictionary
file(GLOB_RECURSE STAGE_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/*.h)
file(GLOB_RECURSE DATAPRODUCT_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/data_products/*.h)
set(ALL_DICT_HEADERS ${STAGE_HEADERS} ${DATAPRODUCT_HEADERS})
list(FILTER ALL_DICT_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Add local and install includes
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
        spdlog_header_only
        nlohmann_json_header_only
        AnalysisPipelineStagesCore::analysis_pipeline_stages_core
        unpacker::common_data_products
        unpacker::nalu_data_products
)

# Gather include directories for ROOT dictionary
set(ROOT_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/LinkDef.h)
set(ROOT_DICTIONARY_INCLUDE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/include")

# Helper function for pulling include paths from a target
function(append_target_includes_to_root_dict target_name)
  get_target_property(INCLUDE_DIRS ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
  if(INCLUDE_DIRS AND NOT INCLUDE_DIRS STREQUAL "${target_name}_NOTFOUND")
    foreach(dir IN LISTS INCLUDE_DIRS)
      if(dir MATCHES "^\\$<BUILD_INTERFACE:(.+)>$")
        set(actual "${CMAKE_MATCH_1}")
        if(EXISTS "${actual}")
          list(APPEND ROOT_DICTIONARY_INCLUDE_OPTIONS "-I${actual}")
        endif()
      elseif(NOT dir MATCHES "^\\$")
        if(EXISTS "${dir}")
          list(APPEND ROOT_DICTIONARY_INCLUDE_OPTIONS "-I${dir}")
        endif()
      endif()
    endforeach()
  endif()
  set(ROOT_DICTIONARY_INCLUDE_OPTIONS ${ROOT_DICTIONARY_INCLUDE_OPTIONS} PARENT_SCOPE)
endfunction()

append_target_includes_to_root_dict(AnalysisPipelineStagesCore::analysis_pipeline_stages_core)
append_target_includes_to_root_dict(unpacker::common_data_products)
append_target_includes_to_root_dict(unpacker::nalu_data_products)

# Generate ROOT dictionary
ROOT_GENERATE_DICTIONARY(G__${PROJECT_NAME}
  ${ALL_DICT_HEADERS}
  LINKDEF ${ROOT_LINKDEF}
  OPTIONS ${ROOT_DICTIONARY_INCLUDE_OPTIONS} -Wno-inconsistent-missing-override
)

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/G__${PROJECT_NAME}.cxx
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

# Alias
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Install logic (only if standalone)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  install(TARGETS ${PROJECT_NAME}
      EXPORT ${PROJECT_NAME}Targets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  if(TARGET spdlog_header_only)
    install(TARGETS spdlog_header_only
      EXPORT ${PROJECT_NAME}Targets
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()

  if(TARGET nlohmann_json_header_only)
    install(TARGETS nlohmann_json_header_only
      EXPORT ${PROJECT_NAME}Targets
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()

  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

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
endif()
