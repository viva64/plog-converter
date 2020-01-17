# 2006-2008 (c) Viva64.com Team
# 2008-2020 (c) OOO "Program Verification Systems"

cmake_minimum_required(VERSION 2.8.12)

if(__RESOURCES_INCLUDED)
    return()
endif()
set(__RESOURCES_INCLUDED TRUE)

include(CMakeParseArguments)

set(RESOURCES_CMAKE_FILE "${CMAKE_CURRENT_LIST_FILE}")

function(resources_generate_array VAR RESOURCE)
    file(READ "${RESOURCE}" TEXT HEX)

    string(LENGTH "${TEXT}" TEXT_LENGTH)
    set(OFFSET 0)
    set(AT_COLUMN 32)

    while (TEXT_LENGTH GREATER 0)
        if (TEXT_LENGTH GREATER ${AT_COLUMN})
            set(LENGTH "${AT_COLUMN}")
        else ()
            set(LENGTH "${TEXT_LENGTH}")
        endif ()

        string(SUBSTRING "${TEXT}" ${OFFSET} ${LENGTH} LINE)
        set(LINES "${LINES}\n${LINE}")

        math(EXPR TEXT_LENGTH "${TEXT_LENGTH} - ${LENGTH}")
        math(EXPR OFFSET "${OFFSET} + ${LENGTH}")
    endwhile ()

    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " LINES "${LINES}")
    string(REGEX REPLACE ", $" "" LINES "${LINES}")
    string(REGEX REPLACE " \n" "\n  " LINES "${LINES}")

    set(${VAR} "${LINES}" PARENT_SCOPE)
endfunction()

macro(resources_parse_arguments)
    set(options)
    set(oneValueArgs HEADER SOURCE)
    set(multipleValuesArgs RESOURCES NAMESPACE)
    cmake_parse_arguments(RESOURCES "${options}" "${oneValueArgs}" "${multipleValuesArgs}" ${ARGN})

    if ("${RESOURCES_HEADER}" STREQUAL "")
        set(RESOURCES_HEADER "resources.h")
    endif ()

    set(RESOURCES_BASENAME "${RESOURCES_HEADER}")
    string(REGEX REPLACE "\\.(h|hpp|hxx)$" "" RESOURCES_BASENAME "${RESOURCES_BASENAME}")

    if ("${RESOURCES_SOURCE}" STREQUAL "")
        set(RESOURCES_SOURCE "${RESOURCES_BASENAME}.cpp")
    endif ()

    set(RESOURCES_SETTINGS "${RESOURCES_BASENAME}.res")
endmacro()

function(resources_generate MODE)
    resources_parse_arguments(${ARGN})

    set(DECL "#include <string>\n")

    foreach (NS ${RESOURCES_NAMESPACE})
        set(DECL "${DECL}\nnamespace ${NS} {")
    endforeach ()

    set(DEF "${DECL}")
    set(DECL "${DECL}\n")

    set(VAR)

    foreach (RESOURCE ${RESOURCES_RESOURCES})
        if ("${VAR}" STREQUAL "")
            set(VAR "${RESOURCE}")
        else ()
            set(DECL "${DECL}\nconst std::string &${VAR}();")

            resources_generate_array(ARRAY "${RESOURCE}")

            set(DEF "${DEF}\n\nstatic const unsigned char ${VAR}_array[] = {${ARRAY}\n};\n")
            set(DEF "${DEF}\nconst std::string &${VAR}() {")
            set(DEF "${DEF}\n  static const std::string res = {reinterpret_cast<const char*>(${VAR}_array), sizeof(${VAR}_array)};")
            set(DEF "${DEF}\n  return res;")
            set(DEF "${DEF}\n}")

            set(VAR)
        endif ()
    endforeach ()

    if (NOT "${VAR}" STREQUAL "")
        message(FATAL_ERROR "Variable ${VAR} is not assigned to any resource")
    endif ()

    set(DECL "${DECL}\n")
    set(DEF "${DEF}\n")

    foreach (NS ${RESOURCES_NAMESPACE})
        set(DECL "${DECL}\n}")
        set(DEF "${DEF}\n}")
    endforeach ()

    set(DECL "#pragma once\n${DECL}")

    set(DECL "// This file was automatically generated by resources.cmake\n\n${DECL}")
    set(DEF "// This file was automatically generated by resources.cmake\n\n${DEF}")

    if ("${MODE}" STREQUAL ALL OR "${MODE}" STREQUAL HEADER)
        file(WRITE "${RESOURCES_HEADER}" "${DECL}")
    endif ()

    if ("${MODE}" STREQUAL ALL OR "${MODE}" STREQUAL SOURCE)
        file(WRITE "${RESOURCES_SOURCE}" "${DEF}")
    endif ()
endfunction()

function(add_resources TARGET)
    resources_parse_arguments(${ARGN})

    if (NOT TARGET "${TARGET}")
        message(FATAL_ERROR "Target ${TARGET} doesn't exist")
    endif ()

    if (NOT EXISTS "${RESOURCES_HEADER}" OR
        NOT EXISTS "${RESOURCES_SOURCE}" OR
        NOT EXISTS "${RESOURCES_SETTINGS}")
        resources_generate(ALL ${ARGN})
    else ()
        file(READ "${RESOURCES_SETTINGS}" PREV_ARGS)
        if (NOT "${ARGN}" STREQUAL "${PREV_ARGS}")
          resources_generate(ALL ${ARGN})
        endif ()
    endif ()

    file(WRITE "${RESOURCES_SETTINGS}" "${ARGN}")

    set(VAR)
    set(DEPENDS)

    foreach (RESOURCE ${RESOURCES_RESOURCES})
        if ("${VAR}" STREQUAL "")
            set(VAR "${RESOURCE}")
        else ()
            list(APPEND DEPENDS "${RESOURCE}")
            set(VAR)
        endif ()
    endforeach ()

    if (NOT "${VAR}" STREQUAL "")
        message(FATAL_ERROR "Variable ${VAR} is not assigned to any resource")
    endif ()

    add_custom_command(
        OUTPUT "${RESOURCES_SOURCE}"
        COMMAND "${CMAKE_COMMAND}"
                -D RESOURCES_CMAKE_SCRIPT="${RESOURCES_SETTINGS}"
                -D RESOURCES_CMAKE_SCRIPT_MODE=SOURCE
                -P "${RESOURCES_CMAKE_FILE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating resources ${RESOURCES_SOURCE}"
        DEPENDS ${DEPENDS}
    )

    add_custom_command(
        OUTPUT "${RESOURCES_HEADER}"
        COMMAND "${CMAKE_COMMAND}"
                -D RESOURCES_CMAKE_SCRIPT="${RESOURCES_SETTINGS}"
                -D RESOURCES_CMAKE_SCRIPT_MODE=HEADER
                -P "${RESOURCES_CMAKE_FILE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating resources ${RESOURCES_HEADER}"
    )

    target_sources("${TARGET}" PRIVATE "${RESOURCES_SOURCE}" "${RESOURCES_HEADER}")
endfunction()

if (NOT "${RESOURCES_CMAKE_SCRIPT}" STREQUAL "")
    file(READ "${RESOURCES_CMAKE_SCRIPT}" ARGS)
    resources_generate(${RESOURCES_CMAKE_SCRIPT_MODE} ${ARGS})
endif ()

