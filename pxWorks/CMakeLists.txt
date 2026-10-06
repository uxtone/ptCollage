project(pxtone-works)

set(PROJECT_SOURCES
    pxCSV2.cpp
    pxDateTime.cpp
    pxDebugLog.cpp
    pxDescriptor.cpp
    pxDirectory.cpp
    pxError.cpp
    pxFile2.cpp
    pxFontParam.cpp
    pxLocalize.cpp
    pxMem.cpp
    pxMouse.cpp
    pxMutex.cpp
    pxOggVorbis.cpp
    pxPalette.cpp
    pxPath.cpp
    pxPlayPCM.cpp
    pxPlaySampler.cpp
    pxRiffWav.cpp
    pxShiftJIS.cpp
    pxStr.cpp
    pxStrT.cpp
    pxSurface.cpp
    pxSurface_png.cpp
#    pxTText.cpp # unused: replaced by uxString.h (uxT / uxDS_from_sjis ...)
    pxUTF8.cpp
    pxWords.cpp
)

set(PROJECT_HEADERS
    pxCSV2.h
    pxDateTime.h
    pxDebugLog.h
    pxDescriptor.h
    pxDirectory.h
    pxError.h
    pxFile2.h
    pxFontParam.h
    pxLocalize.h
    pxMem.h
    pxMouse.h
    pxMutex.h
    pxOggVorbis.h
    pxPalette.h
    pxPath.h
    pxPlayPCM.h
    pxPlaySampler.h
    pxRiffWav.h
    pxShiftJIS.h
    pxStr.h
    pxStrT.h
    pxSurface.h
#    pxTText.h
    pxUTF8.h
    pxWords.h
)

add_pxtone_library(
    ${PROJECT_NAME}
    STATIC
    "${PROJECT_SOURCES};${PROJECT_HEADERS}"
    pxtone::windows
    pxtone::wrapper

    ${VORBISFILE_LIBRARY}
    ${PNG_LIBRARIES} # pxSurface_png.cpp
)

# Export
# ---------------

add_library(pxtone::works ALIAS ${PROJECT_NAME})
