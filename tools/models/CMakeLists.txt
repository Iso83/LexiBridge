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

string(REPLACE "/" "_" MODULE_NAME "tools/models")

set(MODULE_TARGET "${PROJECT_NAME}_${MODULE_NAME}")
set(MODULE_TARGET_SUPPORT "${MODULE_TARGET}_support")
set(MODULE_TARGET_PREPARE "${MODULE_TARGET}_prepare")

add_executable(${MODULE_TARGET}
    ${MODULE_MAIN}
)

add_library(${MODULE_TARGET_SUPPORT} INTERFACE)

target_sources(${MODULE_TARGET_SUPPORT}
    INTERFACE
        ${MODULE_SRC}
        ${MODULE_PRIVATE_HEADERS}
)

add_custom_target(${MODULE_TARGET_PREPARE} ALL
    COMMAND "${CMAKE_COMMAND}"
        "-DLEXIBRIDGE_DATA_DIR=${LEXIBRIDGE_DATA_DIR}"
        "-DLEXIBRIDGE_MODEL_ARCHIVE_DIR=${LEXIBRIDGE_MODEL_ARCHIVE_DIR}"
        "-DLEXIBRIDGE_UNPACK=$<TARGET_FILE:${MODULE_TARGET}>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PrepareModels.cmake"
    DEPENDS 
        ${MODULE_TARGET}
    COMMENT "Prepare verified local NL/EN Bergamot models"
    VERBATIM
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

# ============================================================================
# IDE organization
# ============================================================================

cppcmake_project_set_ide_folder(${MODULE_TARGET} "tools/models")
cppcmake_project_set_ide_folder(${MODULE_TARGET_PREPARE} "tools/models")

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source"
    FILES
        ${MODULE_SRC}
        ${MODULE_PRIVATE_HEADERS}
)

# ============================================================================
# Dependencies
# ============================================================================

target_link_libraries(${MODULE_TARGET_SUPPORT}
    INTERFACE
        ${ZLIB}
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
        Test_${MODULE_TARGET}_Unpacker
    TARGET
        ${MODULE_TARGET_SUPPORT}
    MODULE
        "tools/models"
    SOURCES
        tests/Unpacker.cpp
    INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    COMPILE_DEFINITIONS
        "LEXIBRIDGE_UNPACK_TEST_DIR=\"${PROJECT_BINARY_DIR}/model-unpack-test\""
    LABELS 
        unit
        integrity
)
