project(pxtone-generic)

set(PROJECT_SOURCES
    cls_ExistingWindow.cpp
    if_Generic.cpp
    if_gen_Scroll.cpp
    Japanese.cpp
    KeyControl.cpp
    Menu_History.cpp
    MessageBox.cpp
    OpenVolumeControl.cpp
    pxMidiIn.cpp
    TriggerFile.cpp
)

set(PROEJCT_HEADERS
    cls_ExistingWindow.h
    if_Generic.h
    if_gen_Scroll.h
    Japanese.h
    KeyControl.h
    Menu_History.h
    MessageBox.h
    OpenVolumeControl.h
    pxMidiIn.h
)

add_pxtone_library(
    ${PROJECT_NAME}
    STATIC
    "${PROJECT_SOURCES};${PROEJCT_HEADERS}"
    pxtone::windows
    pxtone::works
)

# Export
# ---------------

add_library(pxtone::generic ALIAS ${PROJECT_NAME})
