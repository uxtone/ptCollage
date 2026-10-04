project(pxtone-tool)

set(PROJECT_RESOURCES
    Script.rc
)

if(WINELIB)
    # A Winelib "DLL" pulled in by ld.so never gets its Win32 imports resolved,
    # so link statically: no DllMain, and add_pxtone_executable merges the resources.
    set(PROJECT_SCOPE STATIC)
    set(PROJECT_SOURCES
        HearSelect.cpp
        Japanese.cpp
    )
else()
    set(PROJECT_SCOPE SHARED)
    set(PROJECT_SOURCES
        HearSelect.cpp
        Japanese.cpp
        Main.cpp

        ${PROJECT_RESOURCES}
    )
endif()

set(PROJECT_HEADERS
    HearSelect.h
    Japanese.h
    pxtoneTool.h
    resource.h
)

add_pxtone_library(
    ${PROJECT_NAME}
    ${PROJECT_SCOPE}
    "${PROJECT_SOURCES};${PROJECT_HEADERS}"
    pxtone::windows
    pxtone::works
    pxtone::xa2
    pxtone::generic
    pxtone::pxtone
)

if(WINELIB)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PXTONETOOL_STATIC)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DPXTONETOOL_EXPORTS)
endif()

# Export
# ---------------

add_library(pxtone::tool ALIAS ${PROJECT_NAME})
