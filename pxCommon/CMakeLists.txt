project(pxtone-common)

add_library(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/pxStdDef.h")

# Consumers do not have to manually include
set_target_properties(${PROJECT_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT "${BUNDLE_BUILD_DIR}" STREQUAL "")
    file(COPY "${CMAKE_SOURCE_DIR}/data/" DESTINATION "${CMAKE_SOURCE_DIR}/${BUNDLE_BUILD_DIR}")
endif()

# Create shorthand for subproject boilerplate
# ---------------

# Common behavior for any binary in our build
macro(target_binary PROJECT)

    # Multi-config generators are too unpredictable to test & maintain code for; use CMAKE_BUILD_TYPE to make a release
    get_cmake_property(MULTI_CONFIG_BUILD GENERATOR_IS_MULTI_CONFIG)
    if(NOT MULTI_CONFIG_BUILD AND NOT "${BUNDLE_BUILD_DIR}" STREQUAL "")
        set(OUT_DIR "${CMAKE_SOURCE_DIR}/${BUNDLE_BUILD_DIR}")
        message("${OUT_DIR}")
        set_target_properties(${PROJECT} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUT_DIR}"
            OPTIMIZE_DEPENDENCIES true
            PCH_WARN_INVALID true
        )
    endif()
endmacro()

# Create a target with all requirements for internal subprojects
macro(add_pxtone_subproject PROJECT)
    target_link_libraries(${PROJECT} PRIVATE
    pxtone::common
    ${ARGN}
  )
    target_binary(${PROJECT})
    set(STDAFX_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx/StdAfx.h")
    if(EXISTS "${STDAFX_HEADER}")
        target_include_directories(${PROJECT} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx")
        target_precompile_headers(${PROJECT} PRIVATE ${STDAFX_HEADER})
    endif()
endmacro()

# Create a target with all requirements for an executable
macro(add_pxtone_executable PROJECT SOURCES)
    if(USE_UNICODE)
        list(APPEND SOURCES "${UTF8_MANIFEST_RC}")
    endif()
    if(WINELIB)
        # winebuild rejects two .res files whose icon ids overlap, and the statically linked
        # pxtoneTool brings its own resources, so compile everything as one script.
        # The executable goes first: wrc lets pxtoneTool's resource.h redefine clashing ids after it.
        set(WINELIB_RC_INPUTS ${SOURCES})
        list(FILTER WINELIB_RC_INPUTS INCLUDE REGEX "\\.rc$")
        set(WINELIB_SOURCES ${SOURCES})
        list(FILTER WINELIB_SOURCES EXCLUDE REGEX "\\.rc$")
        list(TRANSFORM WINELIB_RC_INPUTS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
        if("${ARGN}" MATCHES "pxtone::tool")
            list(APPEND WINELIB_RC_INPUTS "${CMAKE_SOURCE_DIR}/pxtoneTool/Script.rc")
        endif()
        set(WINELIB_RC "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}-merged.rc")
        set(WINELIB_RC_TEXT "")
        foreach(RC_INPUT ${WINELIB_RC_INPUTS})
            file(RELATIVE_PATH RC_INPUT_REL "${CMAKE_SOURCE_DIR}" "${RC_INPUT}")
            string(APPEND WINELIB_RC_TEXT "#include \"${RC_INPUT_REL}\"\n")
        endforeach()
        file(WRITE "${WINELIB_RC}" "${WINELIB_RC_TEXT}")
        set_source_files_properties("${WINELIB_RC}" PROPERTIES OBJECT_DEPENDS "${WINELIB_RC_INPUTS}")
        add_executable(${PROJECT} WIN32 ${WINELIB_SOURCES} "${WINELIB_RC}")
    else()
        add_executable(${PROJECT} WIN32 ${SOURCES})
    endif()
    if(WINELIB)
        target_include_directories(${PROJECT} PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    add_pxtone_subproject(${PROJECT} ${ARGN})
endmacro()

# Create a target with all requirements for an internal library
macro(add_pxtone_library PROJECT SCOPE SOURCES)
    add_library(${PROJECT} ${SCOPE} ${SOURCES})
    set_target_properties(${PROJECT} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
    if(NOT ${SCOPE} STREQUAL "INTERFACE")
        # Consumers do not have to manually include this target's path
        target_include_directories(${PROJECT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
        add_pxtone_subproject(${PROJECT} ${ARGN})
    endif()
endmacro()

# Create a target with all requirements for an internally-managed external dependency
macro(add_bundled_library PROJECT SCOPE SOURCES INCLUDE_DIRS)
    if(WINELIB)
        # These are plain C libraries: build them as native static code. As shared objects they
        # would become Winelib DLLs that ld.so loads behind Wine's back, and with the Windows
        # macros defined they take their MSVC code paths against glibc.
        add_library(${PROJECT} STATIC ${SOURCES})
        target_compile_options(${PROJECT} PRIVATE -U_WIN32 -UWIN32 -U__WIN32__ -U__WIN32 -U_WIN64 -UWIN64 -U__WIN64__ -U__WIN64 -fPIC
            -include "${CMAKE_SOURCE_DIR}/compat/winelib/pxWinelibSetjmp.h")
    else()
        add_library(${PROJECT} ${SCOPE} ${SOURCES})
    endif()
    target_include_directories(${PROJECT} PUBLIC "${INCLUDE_DIRS}")
    target_link_libraries(${PROJECT} PRIVATE ${ARGN})
    if (MSVC)
        target_compile_options(${PROJECT} PRIVATE "/W0")
    else()
        target_compile_options(${PROJECT} PRIVATE "-w")
    endif()
    target_binary(${PROJECT})
endmacro()

# Export
# ---------------

add_library(pxtone::common ALIAS ${PROJECT_NAME})
