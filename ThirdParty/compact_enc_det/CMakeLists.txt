#  2006-2008 (c) Viva64.com Team
#  2008-2020 (c) OOO "Program Verification Systems"
#  2020 (c) PVS-Studio LLC
cmake_minimum_required(VERSION 3.10)

project(ced CXX)

if (NOT PVS_STUDIO_SHARED)
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
endif ()

add_library(ced STATIC
    compact_enc_det/compact_enc_det.cc
    compact_enc_det/compact_enc_det_hint_code.cc
    util/encodings/encodings.cc
    util/languages/languages.cc
)

target_include_directories(ced PUBLIC .)

if(WIN32)
    target_compile_definitions(ced PRIVATE -DUNICODE -D_UNICODE -DSTRICT -DNOMINMAX)
    set(THREADING threadwin)
else()
    set(THREADING thread)
    list(APPEND EXTRA_TARGET_LINK_LIBRARIES -pthread)
    target_link_libraries(ced pthread)
endif()

if (MSVC)
    if (NOT PVS_STUDIO_SHARED)
        foreach (flags CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
            string(REGEX REPLACE "/MD" "/MT" "${flags}" "${${flags}}")
        endforeach ()
    endif ()

    target_compile_options(ced PUBLIC /EHa)

    target_compile_definitions(ced PUBLIC -D_CRT_SECURE_NO_WARNINGS
                                           -D_HAS_ITERATOR_DEBUGGING=0
                                           -D_SECURE_SCL=0)
else ()
    target_compile_options(ced PUBLIC -Wno-narrowing -Wno-overflow -g3)

    if (NOT PVS_STUDIO_SHARED AND NOT APPLE)
        target_link_libraries(ced -static)
    endif ()
endif ()
