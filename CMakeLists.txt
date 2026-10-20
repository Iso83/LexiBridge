cmake_minimum_required(VERSION 4.0)

project(LexiBridge
    VERSION 0.1.0
    LANGUAGES C CXX
)

# =========================================================
# Project configuration
# =========================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(LEXIBRIDGE_DATA_DIR "${PROJECT_BINARY_DIR}/data" CACHE PATH "Local translation model and dictionary directory")
set(LEXIBRIDGE_MODEL_ARCHIVE_DIR "" CACHE PATH "Optional directory containing the six downloaded model gzip files")
set(LEXIBRIDGE_WORDNET_ARCHIVE "" CACHE FILEPATH "Optional local OEWN 2025 WNDB zip archive")

# =========================================================
# CppCMake
# =========================================================

include(cmake/Dependencies.cmake)

dependencies_fetch_cppcmake()

cppcmake_project_initialize()

if(CPPCMAKE_PROJECT_ALREADY_REGISTERED)
    return()
endif()

cppcmake_githook_precommit()
cppcmake_version_generate()
cppcmake_test_setup()

# =========================================================
# Dependencies
# =========================================================

cppcmake_dependency_try_local_packages()

cppcmake_gitsubmodule_init_missing_targets(
    WORKING_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}"

    PATH
        "extern/CppDependencies"
)

include(extern/CppDependencies/CppDependencies.cmake)

cppdependencies_zlib(ZLIB STATIC)
cppdependencies_cli11(CLI11)
cppdependencies_bergamot(BERGAMOT)

# =========================================================
# Modules
# =========================================================

cppcmake_module_add(core/translator)
cppcmake_module_add(tools/models)
cppcmake_module_add(tools/wordnet)
cppcmake_module_add(apps/translator)
