project(pxtone-config)

set(PROJECT_SOURCES
    ptConfig_Build.cpp
    ptConfig.cpp
    ptConfig_DlgCtrl.cpp
    ptConfig_Font.cpp
    ptConfig_MIDI.cpp
    ptConfig_Stream.cpp
)

set(PROJECT_HEADERS
    ptConfig.h
    ptConfig_Build.h
    ptConfig_DlgCtrl.h
    ptConfig_Font.h
    ptConfig_MIDI.h
    ptConfig_Stream.h
)

add_pxtone_library(
    ${PROJECT_NAME}
    STATIC
    "${PROJECT_SOURCES};${PROJECT_HEADERS}"
    pxtone::wrapper
    pxtone::windows
    pxtone::works
)

# Export
# ---------------

add_library(pxtone::config ALIAS ${PROJECT_NAME})
