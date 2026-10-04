project(pxtone-windows)

set(PROJECT_SOURCES
    pxwAlteration.cpp
    pxwAppEtc.cpp
    pxwDirectory.cpp
    pxwDlg_SelFile.cpp
    pxwDx09Draw.cpp
    pxwFile.cpp
    pxwFilePath.cpp
    pxwGlyph1.cpp
    pxwMutex.cpp
    pxwPathDialog.cpp
    pxwRuntime.cpp
    pxwTextOverride.cpp
    pxwThread.cpp
    pxwUTF8.cpp
    pxwWindowRect.cpp
    pxwXA2stream.cpp
    pxwXA2unit.cpp
    pxwXAudio2.cpp
    pxwXAudio2Keep.cpp
)

set(PROJECT_HEADERS
    pxwAlteration.h
    pxwAppEtc.h
    pxwDirectory.h
    pxwDlg_SelFile.h
    pxwDx09Draw.h
    pxwEntryPoint.h
    pxwFile.h
    pxwFilePath.h
    pxwGlyph1.h
    pxwMutex.h
    pxwPathDialog.h
    pxwRuntime.h
    pxwTextOverride.h
    pxwThread.h
    pxwUTF8.h
    pxwWindowRect.h
    pxwXA2stream.h
    pxwXA2unit.h
    pxwXAudio2.h
    pxwXAudio2Keep.h
)

set(WINDOWS_LIBS
  version
  comctl32
  dxguid
  shlwapi
  winmm
  imm32
  d3dx9
  d3d9
)
if(WINELIB)
  list(APPEND WINDOWS_LIBS
    ole32
    uuid
    xaudio2_8
  )
else()
  list(APPEND WINDOWS_LIBS
    # Windows' XAudio2 relies on this GUID library that Winelib doesn't ship
    # Winelib doesn't need it as FAudio (the XAudio2 driver) does not require it
    ksguid
  )
endif()

add_pxtone_library(
    ${PROJECT_NAME}
    STATIC
    "${PROJECT_SOURCES};${PROJECT_HEADERS}"
    pxtone::works

    ${WINDOWS_LIBS}
)

if(WINELIB)
  # XAudio2Create resolves against Wine's xaudio2_8 import library
  target_compile_definitions(${PROJECT_NAME} PRIVATE pxwXAUDIO2_ENABLE)
endif()

# Export
# ---------------

add_library(pxtone::windows ALIAS ${PROJECT_NAME})
