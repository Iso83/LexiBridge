# ============================================================================
# Source files
# ============================================================================

file(GLOB_RECURSE MODULE_SRC CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE MODULE_PRIVATE_HEADERS CONFIGURE_DEPENDS
    src/*.h
)

set(MODULE_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp")
list(REMOVE_ITEM MODULE_SRC "${MODULE_MAIN}")

# ============================================================================
# Target
# ============================================================================

string(REPLACE "/" "_" MODULE_NAME "apps/translator")

set(MODULE_TARGET "${PROJECT_NAME}_${MODULE_NAME}")
set(MODULE_TARGET_SUPPORT "${MODULE_TARGET}_support")

add_executable(${MODULE_TARGET}
    ${MODULE_MAIN}
)

set_target_properties(${MODULE_TARGET} PROPERTIES OUTPUT_NAME "translator")

add_library(${MODULE_TARGET_SUPPORT} INTERFACE)

target_sources(${MODULE_TARGET_SUPPORT}
    INTERFACE
        ${MODULE_SRC}
        ${MODULE_PRIVATE_HEADERS}
)

# ============================================================================
# Build configuration
# ============================================================================

cppcmake_version_target(${MODULE_TARGET})
cppcmake_helper_include_target(${MODULE_TARGET})

target_include_directories(${MODULE_TARGET}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(${MODULE_TARGET}
    PUBLIC
        cxx_std_20
)

target_compile_features(${MODULE_TARGET_SUPPORT}
    INTERFACE
        cxx_std_20
)

target_compile_definitions(${MODULE_TARGET_SUPPORT}
    INTERFACE
        # Marian's older bundled CLI11 must not share header-only symbols with this
        # application's CLI11, including when Bergamot comes from a prebuilt package.
        CLI=LexiBridgeCLI
)

# ============================================================================
# IDE organization
# ============================================================================

cppcmake_project_set_ide_folder(${MODULE_TARGET} "apps/translator")

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source"
    FILES
        ${MODULE_SRC}
        ${MODULE_PRIVATE_HEADERS}
        src/Main.cpp
)

# ============================================================================
# Dependencies
# ============================================================================

target_link_libraries(${MODULE_TARGET_SUPPORT}
    INTERFACE
        ${CLI11}
        ${PROJECT_NAME}::core_translator
)

target_link_libraries(${MODULE_TARGET}
    PRIVATE
        ${MODULE_TARGET_SUPPORT}
)

cppcmake_dependency_copy_runtime(${MODULE_TARGET})

# ============================================================================
# Tests
# ============================================================================

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Application
    TARGET
        ${MODULE_TARGET_SUPPORT}
    MODULE
        "apps/translator"
    SOURCES
        tests/Application.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "LEXIBRIDGE_TEST_DATA_DIR=\"${LEXIBRIDGE_DATA_DIR}\""
        "LEXIBRIDGE_TEST_INPUT=\"${CMAKE_CURRENT_BINARY_DIR}/input-test.txt\""
    LABELS
        app
        integration
        models
        offline
)
