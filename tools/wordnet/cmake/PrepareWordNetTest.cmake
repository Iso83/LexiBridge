cmake_minimum_required(VERSION 4.0)

if(NOT LEXIBRIDGE_WORDNET_TEST_DIR OR NOT EXISTS "${LEXIBRIDGE_WORDNET_TEST_ARCHIVE}")
    message(FATAL_ERROR 
        "Build LexiBridge_tools_wordnet_prepare before running this test"
    )
endif()

set(scratch "${LEXIBRIDGE_WORDNET_TEST_DIR}/path with spaces")
file(REMOVE_RECURSE "${scratch}")
file(MAKE_DIRECTORY "${scratch}")
set(prepare "${CMAKE_CURRENT_LIST_DIR}/PrepareWordNet.cmake")

function(run_prepare data archive expected)
    execute_process(COMMAND "${CMAKE_COMMAND}"
        "-DLEXIBRIDGE_SOURCE_DIR=${LEXIBRIDGE_SOURCE_DIR}"
        "-DLEXIBRIDGE_DATA_DIR=${data}"
        "-DLEXIBRIDGE_WORDNET_ARCHIVE=${archive}"
        -P "${prepare}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors
    )

    if(expected STREQUAL "success")
        if(NOT result STREQUAL "0")
            message(FATAL_ERROR
                "Preparation failed: ${output}\n${errors}"
            )
        endif()
    elseif(result STREQUAL "0" OR NOT errors MATCHES "${expected}")
        message(FATAL_ERROR
            "Expected ${expected}, got ${result}: ${output}\n${errors}"
        )
    endif()
endfunction()

# Real archive, new data directory; preparation must also retain attribution.
run_prepare("${scratch}/data" "${LEXIBRIDGE_WORDNET_TEST_ARCHIVE}" success)
foreach(name IN ITEMS data.noun data.verb data.adj data.adv LICENSE.md WNDB_License.txt ATTRIBUTION.txt)
    if(NOT EXISTS "${scratch}/data/wordnet/${name}")
        message(FATAL_ERROR
            "Missing prepared WordNet file: ${name}"
        )
    endif()
endforeach()

# Reuse must work without the source archive or any download, even without cache.
set(cache "${scratch}/data/cache/wordnet-2025/english-wordnet-2025-index.sense-fixed.zip")
file(REMOVE "${cache}")
run_prepare("${scratch}/data" "${scratch}/missing.zip" success)

# Broken data is repaired from a verified local archive, then from the cache.
file(WRITE "${scratch}/data/wordnet/data.verb" "broken")
run_prepare("${scratch}/data" "${LEXIBRIDGE_WORDNET_TEST_ARCHIVE}" success)
file(SHA256 "${scratch}/data/wordnet/data.verb" actual)
if(NOT actual STREQUAL "2ef76318838583e5cb0d04d019fd7200c629d7aed8cc5cfea41122d3d67c866c")
    message(FATAL_ERROR
        "Broken dictionary was not repaired"
    )
endif()
file(REMOVE "${scratch}/data/wordnet/data.adj")
run_prepare("${scratch}/data" "${scratch}/missing.zip" success)

# No network is used for either error case; invalid local input cannot be installed.
run_prepare("${scratch}/absent" "${scratch}/missing.zip" "Missing local WordNet archive")
file(WRITE "${scratch}/broken.zip" "not a dictionary archive")
run_prepare("${scratch}/invalid" "${scratch}/broken.zip" "WordNet archive SHA-256 mismatch")
if(EXISTS "${scratch}/invalid/wordnet/data.noun")
    message(FATAL_ERROR
        "Unverified dictionary was installed"
    )
endif()
file(REMOVE_RECURSE "${scratch}")
