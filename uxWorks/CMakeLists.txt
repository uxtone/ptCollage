project(pxtone-works2)

set(PROJECT_SOURCES
    uxWindowMenu.cpp
)

set(PROJECT_HEADERS
    uxWindowMenu.h
)

add_pxtone_library(
    ${PROJECT_NAME}
    STATIC
    "${PROJECT_SOURCES};${PROJECT_HEADERS}"
    pxtone::windows
    pxtone::wrapper
)

# Export
# ---------------

add_library(pxtone::works2 ALIAS ${PROJECT_NAME})
