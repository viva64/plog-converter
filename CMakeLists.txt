#  2006-2008 (c) Viva64.com Team
#  2008-2019 (c) OOO "Program Verification Systems"
cmake_minimum_required(VERSION 3.10)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
set(CMAKE_CXX_STANDARD 17)

project(plog-converter CXX)

if (NOT PVS_STUDIO_SHARED)
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
endif ()

add_library(plog STATIC
    application.cpp
    configparser.cpp
    iworker.cpp
    logparserworker.cpp
    messagefilter.cpp
    messageparser.cpp
    outputfactory.cpp
    warning.cpp
    utils.cpp
    utils.h
    configs.h
    ioutput.h
    ioutput.cpp
    multipleoutput.h
    multipleoutput.cpp
    charmap.cpp

    Formats/htmloutput.cpp
    Formats/htmloutput.h
    Formats/xmloutput.cpp
    Formats/xmloutput.h
    Formats/csvoutput.cpp
    Formats/csvoutput.h
    Formats/errorfileoutput.cpp
    Formats/errorfileoutput.h
    Formats/tasklistoutput.cpp
    Formats/tasklistoutput.h
    Formats/simplehtml.cpp
    Formats/simplehtml.h

    ThirdParty/args.hxx
)

target_include_directories(plog PUBLIC . ThirdParty "${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(ThirdParty/compact_enc_det)
target_link_libraries(plog ced)

add_executable(plog-converter main.cpp)
target_link_libraries(plog-converter plog)

add_executable(plog-converter-test test.cpp)
target_link_libraries(plog-converter-test plog)

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

    target_compile_definitions(plog PUBLIC -D_CRT_SECURE_NO_WARNINGS
                                           -D_HAS_ITERATOR_DEBUGGING=0
                                           -D_SECURE_SCL=0)
else ()
    target_compile_options(plog PRIVATE -Wall -Wextra -Wold-style-cast -Werror -pedantic)

    if (NOT PVS_STUDIO_SHARED AND NOT APPLE)
        target_link_libraries(plog -static)
    endif ()
endif ()

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
    JQueryJs            "Resources/jquery-3.2.1.min.js"
    HighlightCodeCss    "Resources/highlight.css"
    HighlightPackJs     "Resources/highlight.pack.js"
    HighlightLineJs     "Resources/highlightjs-line-numbers.js"
    AscGif              "Resources/asc.gif"
    DescGif             "Resources/desc.gif"
    SortGif             "Resources/sort.gif"
    LogoPng             "Resources/logo.png"
)

enable_testing()
add_test(NAME plog-converter COMMAND plog-converter-test)
