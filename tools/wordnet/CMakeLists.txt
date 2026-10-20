# ============================================================================
# Target
# ============================================================================

string(REPLACE "/" "_" MODULE_NAME "tools/wordnet")

set(MODULE_TARGET "${PROJECT_NAME}_${MODULE_NAME}")
set(MODULE_TARGET_PREPARE "${MODULE_TARGET}_prepare")

add_custom_target(${MODULE_TARGET_PREPARE} ALL
    COMMAND "${CMAKE_COMMAND}"
        "-DLEXIBRIDGE_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
        "-DLEXIBRIDGE_DATA_DIR=${LEXIBRIDGE_DATA_DIR}"
        "-DLEXIBRIDGE_WORDNET_ARCHIVE=${LEXIBRIDGE_WORDNET_ARCHIVE}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PrepareWordNet.cmake"
    COMMENT "Prepare verified Open English WordNet 2025 dictionary"
    VERBATIM
)

# ============================================================================
# IDE organization
# ============================================================================

cppcmake_project_set_ide_folder(${MODULE_TARGET_PREPARE} "tools/wordnet")

# ============================================================================
# Tests
# ============================================================================

if(BUILD_TESTING AND LexiBridge_BUILD_TESTS)
    add_test(NAME Test_${MODULE_TARGET}_Preparation
        COMMAND "${CMAKE_COMMAND}"
            "-DLEXIBRIDGE_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
            "-DLEXIBRIDGE_WORDNET_TEST_ARCHIVE=${LEXIBRIDGE_DATA_DIR}/cache/wordnet-2025/english-wordnet-2025-index.sense-fixed.zip"
            "-DLEXIBRIDGE_WORDNET_TEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/preparation-test"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PrepareWordNetTest.cmake"
    )
    set_tests_properties(Test_${MODULE_TARGET}_Preparation PROPERTIES LABELS "integration;wordnet;offline")
endif()
