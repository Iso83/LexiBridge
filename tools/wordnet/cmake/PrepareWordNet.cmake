cmake_minimum_required(VERSION 4.0)

if(NOT LEXIBRIDGE_SOURCE_DIR)
    message(FATAL_ERROR 
        "LEXIBRIDGE_SOURCE_DIR must point to the LexiBridge source directory"
    )
endif()

if(NOT LEXIBRIDGE_DATA_DIR)
    message(FATAL_ERROR
        "LEXIBRIDGE_DATA_DIR is required"
    )
endif()

# Release: OEWN 2025, corrected WNDB bundle published 2026-06-26.
set(archive_name english-wordnet-2025-index.sense-fixed.zip)
set(archive_hash 38b16326159f51853626b7d24a44c453fa88ab33f06fce5ec8fc5996d1c2be93)
set(names data.noun data.verb data.adj data.adv)
set(hashes
    97c1c4e4f323e2b29bba5ac52b64aca5b91df9f7661dbc65cf64f28ea36e98d3
    2ef76318838583e5cb0d04d019fd7200c629d7aed8cc5cfea41122d3d67c866c
    98651304f90602352120dffdd27dfeb8a3fa4dadfc712391934ad5f466601059
    c40c0bf2eb0c0ba682663a1ad8f62c4eb7cd581189a8f19368b93c6dda8e04d7
)

function(_lexibridge_wordnet_verify directory result)
    foreach(name hash IN ZIP_LISTS names hashes)
        if(NOT EXISTS "${directory}/${name}")
            set(${result} FALSE PARENT_SCOPE)
            return()
        endif()
        file(SHA256 "${directory}/${name}" actual)
        if(NOT actual STREQUAL hash)
            set(${result} FALSE PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${result} TRUE PARENT_SCOPE)
endfunction()

set(directory "${LEXIBRIDGE_DATA_DIR}/wordnet")
set(cache "${LEXIBRIDGE_DATA_DIR}/cache/wordnet-2025")
file(MAKE_DIRECTORY "${cache}" "${directory}")
file(LOCK "${cache}/prepare.lock" GUARD PROCESS TIMEOUT 120)
_lexibridge_wordnet_verify("${directory}" ready)
if(NOT ready)
    set(archive "${cache}/${archive_name}")
    set(actual "")
    if(EXISTS "${archive}")
        file(SHA256 "${archive}" actual)
    endif()
    if(NOT actual STREQUAL archive_hash)
        if(LEXIBRIDGE_WORDNET_ARCHIVE)
            if(NOT EXISTS "${LEXIBRIDGE_WORDNET_ARCHIVE}")
                message(FATAL_ERROR 
                    "Missing local WordNet archive: ${LEXIBRIDGE_WORDNET_ARCHIVE}"
                )
            endif()
            file(COPY_FILE "${LEXIBRIDGE_WORDNET_ARCHIVE}" "${archive}.download")
        else()
            file(DOWNLOAD
                "https://github.com/globalwordnet/english-wordnet/releases/download/2025-edition/${archive_name}"
                "${archive}.download"
                TLS_VERIFY ON STATUS status SHOW_PROGRESS TIMEOUT 600 INACTIVITY_TIMEOUT 60
            )
            list(GET status 0 code)
            if(NOT code EQUAL 0)
                file(REMOVE "${archive}.download")
                message(FATAL_ERROR
                    "Cannot download WordNet: ${status}"
                )
            endif()
        endif()
        file(SHA256 "${archive}.download" actual)
        if(NOT actual STREQUAL archive_hash)
            file(REMOVE "${archive}.download")
            message(FATAL_ERROR 
                "WordNet archive SHA-256 mismatch; expected OEWN 2025 corrected WNDB zip"
            )
        endif()
        file(RENAME "${archive}.download" "${archive}")
    endif()

    set(staging "${cache}/staging")
    file(REMOVE_RECURSE "${staging}")
    file(ARCHIVE_EXTRACT INPUT "${archive}" DESTINATION "${staging}" PATTERNS "oewn2025/data.*")
    _lexibridge_wordnet_verify("${staging}/oewn2025" valid)
    if(NOT valid)
        message(FATAL_ERROR 
            "Extracted WordNet data failed SHA-256 verification"
        )
    endif()
    foreach(name IN LISTS names)
        file(RENAME "${staging}/oewn2025/${name}" "${directory}/${name}")
    endforeach()
    file(REMOVE_RECURSE "${staging}")
endif()

file(COPY "${LEXIBRIDGE_SOURCE_DIR}/doc/wordnet/LICENSE.md"
          "${LEXIBRIDGE_SOURCE_DIR}/doc/wordnet/WNDB_License.txt"
          "${LEXIBRIDGE_SOURCE_DIR}/doc/wordnet/ATTRIBUTION.txt"
     DESTINATION "${directory}"
)

message(STATUS 
    "Open English WordNet 2025 ready: ${directory}"
)
