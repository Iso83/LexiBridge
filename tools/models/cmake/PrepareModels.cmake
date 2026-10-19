cmake_minimum_required(VERSION 4.0)

if(NOT LEXIBRIDGE_DATA_DIR OR NOT EXISTS "${LEXIBRIDGE_UNPACK}")
    message(FATAL_ERROR
        "Run the LexiBridge_tools_models_prepare build target"
    )
endif()

function(lexibridge_prepare_model_asset direction export_name upstream_name local_name expected_hash)
    set(source "${direction}/${export_name}/exported/${upstream_name}.gz")
    set(url "https://storage.googleapis.com/moz-fx-translations-data--303e-prod-translations-data/models/${source}")
    set(directory "${LEXIBRIDGE_DATA_DIR}/${direction}")
    set(cache "${LEXIBRIDGE_DATA_DIR}/cache/${direction}/${export_name}")
    file(MAKE_DIRECTORY "${directory}" "${cache}")
    set(output "${directory}/${local_name}")
    if(EXISTS "${output}" AND EXISTS "${output}.sha256" AND EXISTS "${output}.crc32")
        file(SHA256 "${output}" actual_hash)
        file(READ "${output}.sha256" recorded_hash)
        file(READ "${output}.crc32" checksum)
        string(STRIP "${checksum}" checksum)
        if("${source}:${actual_hash}:${checksum}" STREQUAL recorded_hash
            AND (expected_hash STREQUAL "" OR actual_hash STREQUAL expected_hash))
            return()
        endif()
    endif()

    set(compressed "${cache}/${upstream_name}.gz")
    set(reuse FALSE)
    if(EXISTS "${compressed}" AND EXISTS "${compressed}.sha256")
        file(SHA256 "${compressed}" actual_hash)
        file(READ "${compressed}.sha256" recorded_hash)
        if("${source}:${actual_hash}" STREQUAL recorded_hash)
            set(reuse TRUE)
        endif()
    endif()
    if(NOT reuse)
        if(LEXIBRIDGE_MODEL_ARCHIVE_DIR)
            set(archive "${LEXIBRIDGE_MODEL_ARCHIVE_DIR}/${upstream_name}.gz")
            if(NOT EXISTS "${archive}")
                message(FATAL_ERROR
                    "Missing local model archive: ${archive}"
                )
            endif()
            file(COPY_FILE "${archive}" "${compressed}.download")
        else()
            file(DOWNLOAD "${url}" "${compressed}.download"
                TLS_VERIFY ON STATUS status SHOW_PROGRESS
                TIMEOUT 600 INACTIVITY_TIMEOUT 60
            )
            list(GET status 0 code)
            if(NOT code EQUAL 0)
                file(REMOVE "${compressed}.download")
                message(FATAL_ERROR
                    "Cannot download ${url}\n${status}"
                )
            endif()
        endif()
        file(SHA256 "${compressed}.download" actual_hash)
        file(RENAME "${compressed}.download" "${compressed}")
        file(WRITE "${compressed}.sha256" "${source}:${actual_hash}")
    endif()

    execute_process(COMMAND "${LEXIBRIDGE_UNPACK}" "${compressed}" "${output}.staging"
        RESULT_VARIABLE result)
    if(NOT result STREQUAL "0")
        file(REMOVE "${compressed}" "${compressed}.sha256" "${output}.staging" "${output}.staging.crc32")
        message(FATAL_ERROR
            "Cannot unpack ${source}: ${result}"
        )
    endif()
    file(SHA256 "${output}.staging" hash)
    if(NOT expected_hash STREQUAL "" AND NOT hash STREQUAL expected_hash)
        file(REMOVE "${compressed}" "${compressed}.sha256" "${output}.staging" "${output}.staging.crc32")
        message(FATAL_ERROR
            "Uncompressed SHA-256 mismatch for ${source}"
        )
    endif()
    file(READ "${output}.staging.crc32" checksum)
    string(STRIP "${checksum}" checksum)
    file(RENAME "${output}.staging" "${output}")
    file(RENAME "${output}.staging.crc32" "${output}.crc32")
    file(WRITE "${output}.sha256" "${source}:${hash}:${checksum}")
endfunction()

# Mozilla's model registry, generated 2026-10-18. Pin the release exports rather
# than following a mutable registry selection. Hashes refer to unpacked weights.
# Vocabulary/shortlist hashes pin the corresponding validated release bundle.
set(nlen_export retrain_hr_JbTCb42HREGvK8d6aOCjFQ)
set(ennl_export retrain_hr_fix_names_fR8XhBiBQniIbi4I_ebJ2Q)
lexibridge_prepare_model_asset(nl-en ${nlen_export} model.nlen.intgemm.alphas.bin model.bin
    ff987741ff15a5d94fff16cbef28f2b0a85a6e9547d982fad60ac09d16d11270)
lexibridge_prepare_model_asset(nl-en ${nlen_export} vocab.nlen.spm vocab.spm
    c09588ca5662b230ed65bd39a07f58d9f7dd21c3a19e4aeb1bff39bf3b297c56)
lexibridge_prepare_model_asset(nl-en ${nlen_export} lex.50.50.nlen.s2t.bin lex.bin
    811ac8f9dcbd12706e2d73a7057a7bbe6b9f0607d15170ffe9439977e8229b41)
lexibridge_prepare_model_asset(en-nl ${ennl_export} model.ennl.intgemm.alphas.bin model.bin
    7e8402da61972dc2fdcf7147c7f1e12f7916a6d9349514ee7a731288357575a9)
lexibridge_prepare_model_asset(en-nl ${ennl_export} vocab.ennl.spm vocab.spm
    c09588ca5662b230ed65bd39a07f58d9f7dd21c3a19e4aeb1bff39bf3b297c56)
lexibridge_prepare_model_asset(en-nl ${ennl_export} lex.50.50.ennl.s2t.bin lex.bin
    bb4922718f8ca6d746ad090c0a88110a664dc050ee3f311bce33698b4e1679e9)

foreach(direction IN ITEMS nl-en en-nl)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/model-config.yml"
        "${LEXIBRIDGE_DATA_DIR}/${direction}/config.yml" COPYONLY)
endforeach()

message(STATUS
    "Local translation data ready: ${LEXIBRIDGE_DATA_DIR}"
)
