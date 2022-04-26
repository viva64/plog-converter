#  2006-2008 (c) Viva64.com Team
#  2008-2020 (c) OOO "Program Verification Systems"
#  2020-2022 (c) PVS-Studio LLC

cmake_minimum_required(VERSION 3.11)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
set(CMAKE_CXX_STANDARD 17)

project(plog-converter CXX)

if (NOT PVS_STUDIO_SHARED)
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
endif ()

add_library(plog STATIC
    suppress_base.suppress
    application.cpp
    application.h
    charmap.cpp
    charmap.h
    configparser.cpp
    configparser.h
    configs.h
    ioutput.cpp
    ioutput.h
    iworker.cpp
    JsonUtils.h
    logparserworker.cpp
    logparserworker.h
    messagefilter.cpp
    messagefilter.h
    messageparser.cpp
    messageparser.h
    multipleoutput.cpp
    multipleoutput.h
    outputfactory.cpp
    outputfactory.h
    sourceroottransformer.cpp
    sourceroottransformer.h
    suppressfilter.cpp
    suppressfilter.h
    utils.cpp
    utils.h
    warning.cpp
    warning.h

    Formats/csvoutput.cpp
    Formats/csvoutput.h
    Formats/errorfileoutput.cpp
    Formats/errorfileoutput.h
	Formats/errorfileverboseoutput.cpp
    Formats/errorfileverboseoutput.h
    Formats/htmloutput.cpp
    Formats/htmloutput.h
    Formats/jsonoutput.h
    Formats/jsonoutput.cpp
    Formats/misracomplianceoutput.cpp
    Formats/misracomplianceoutput.h
    Formats/sarifoutput.cpp
    Formats/sarifoutput.h
    Formats/sarifvscodeoutput.cpp
    Formats/sarifvscodeoutput.h
    Formats/simplehtml.cpp
    Formats/simplehtml.h
    Formats/tasklistoutput.cpp
    Formats/tasklistoutput.h
    Formats/tasklistverboseoutput.cpp
    Formats/tasklistverboseoutput.h
    Formats/teamcityoutput.cpp
    Formats/teamcityoutput.h
    Formats/xmloutput.cpp
    Formats/xmloutput.h

    ThirdParty/args.hxx
    ThirdParty/backward.hpp
    ThirdParty/json.hpp
    ThirdParty/strnatcmp/strnatcmp.cpp
    ThirdParty/strnatcmp/strnatcmp.h
)

target_include_directories(plog PUBLIC . ThirdParty "${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(ThirdParty/compact_enc_det)
target_link_libraries(plog ced $ENV{CXX_LIBS_STATIC})

add_executable(plog-converter main.cpp suppress_base.suppress)
target_link_libraries(plog-converter plog)

add_executable(plog-converter-test test.cpp suppress_base.suppress)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
include(ThirdParty/GetGoogletest.cmake)

if (NOT WIN32 AND NOT APPLE AND NOT PVS_STUDIO_SHARED)
    # fix weak symbols
    # https://stackoverflow.com/questions/35116327/when-g-static-link-pthread-cause-segmentation-fault-why
    target_link_libraries(plog-converter-test plog gtest gtest_main -Wl,--whole-archive,-lrt,-lpthread,--no-whole-archive)
else ()
    target_link_libraries(plog-converter-test plog gtest gtest_main)
endif ()

if (WIN32)
    install(TARGETS plog-converter DESTINATION .)
else ()
    install(TARGETS plog-converter DESTINATION bin)
endif ()

if (MSVC)
    if (NOT PVS_STUDIO_SHARED)
        foreach (flags CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
            string(REGEX REPLACE "/MD" "/MT" "${flags}" "${${flags}}")
        endforeach ()
    endif ()

    target_compile_options(plog PUBLIC /EHa)

    target_compile_definitions(plog-converter-test PUBLIC -D_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING)
else ()
    target_compile_options(plog PRIVATE -Wall -Wextra -Wold-style-cast -Werror -pedantic -g3)
    target_compile_options(plog-converter PRIVATE -Wall -Wextra -pedantic -g3)

    if (NOT PVS_STUDIO_SHARED AND NOT APPLE)
        target_link_libraries(plog -static)
    endif ()
endif ()

if(WIN32)
    add_custom_command(TARGET plog-converter POST_BUILD
	                   COMMAND cmake -E tar -c $<TARGET_PDB_FILE:plog-converter>.zst --zstd -- $<TARGET_PDB_FILE:plog-converter>)
elseif (APPLE)
    add_custom_command(TARGET plog-converter POST_BUILD
                       COMMAND rm -rf $<TARGET_FILE_NAME:plog-converter>*.dSYM*
                       COMMAND dsymutil -o $<TARGET_FILE_NAME:plog-converter>.dSYM $<TARGET_FILE:plog-converter>
                       COMMAND cmake -E tar c $<TARGET_FILE_NAME:plog-converter>.dSYM.zst --zstd -- $<TARGET_FILE_NAME:plog-converter>.dSYM
                       COMMAND strip $<TARGET_FILE:plog-converter>)

    target_link_options(plog-converter PRIVATE
                        -Wl,-object_path_lto,$<TARGET_FILE_NAME:plog-converter>.lto.o)
elseif (UNIX)
    add_custom_command(TARGET plog-converter POST_BUILD
                       COMMAND rm -rf $<TARGET_FILE_NAME:plog-converter>*.dwarf $<TARGET_FILE_NAME:plog-converter>*.dwarf.zst
                       COMMAND objcopy --only-keep-debug $<TARGET_FILE:plog-converter> $<TARGET_FILE_NAME:plog-converter>.dwarf
                       COMMAND strip $<TARGET_FILE:plog-converter>
                       COMMAND objcopy --add-gnu-debuglink=$<TARGET_FILE_NAME:plog-converter>.dwarf $<TARGET_FILE:plog-converter>
					   COMMAND cmake -E tar c $<TARGET_FILE:plog-converter>.dwarf.zst --zstd -- $<TARGET_FILE:plog-converter>.dwarf) 
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if (WIN32)
        set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/PVS-Studio" CACHE PATH "CMake prefix" FORCE)
    elseif (NOT APPLE)
        set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "CMake prefix" FORCE)
    endif ()
endif ()

include(resources.cmake)
add_resources(
    plog
    HEADER "${CMAKE_CURRENT_BINARY_DIR}/resources.h"
    NAMESPACE PlogConverter Resources
    RESOURCES
    SortJs              "Resources/sort.js"
    StyleCss            "Resources/style.css"
    JQueryJs            "Resources/jquery-3.5.1.min.js"
    HighlightCodeCss    "Resources/highlight.css"
    HighlightPackJs     "Resources/highlight.pack.js"
    HighlightLineJs     "Resources/highlightjs-line-numbers.js"
    AscGif              "Resources/asc.gif"
    DescGif             "Resources/desc.gif"
    SortGif             "Resources/sort.gif"
    LogoPng             "Resources/logo.png"
    LogoMisra           "Resources/logomisra.png"
)

gtest_discover_tests(plog-converter-test)
