project(pxtone-engine LANGUAGES CXX)

set(PROJECT_SOURCES
    pxtnData.cpp
    pxtnDelay.cpp
    pxtnError.cpp
    pxtnEvelist.cpp
    pxtnMaster.cpp
    pxtnMem.cpp
    pxtnOverDrive.cpp
    pxtnPulse_Frequency.cpp
    pxtnPulse_Noise.cpp
    pxtnPulse_NoiseBuilder.cpp
    pxtnPulse_Oggv.cpp
    pxtnPulse_Oscillator.cpp
    pxtnPulse_PCM.cpp
    pxtnService.cpp
    pxtnService_moo.cpp
    pxtnText.cpp
    pxtnUnit.cpp
    pxtnWoice.cpp
    pxtnWoicePTV.cpp
    pxtnWoice_io.cpp
    pxtoneNoise.cpp
)

set(PROJECT_HEADERS
    pxtn.h
    pxtnData.h
    pxtnDelay.h
    pxtnError.h
    pxtnEvelist.h
    pxtnMaster.h
    pxtnMax.h
    pxtnMem.h
    pxtnOverDrive.h
    pxtnPulse_Frequency.h
    pxtnPulse_Noise.h
    pxtnPulse_NoiseBuilder.h
    pxtnPulse_Oggv.h
    pxtnPulse_Oscillator.h
    pxtnPulse_PCM.h
    pxtnService.h
    pxtnText.h
    pxtnUnit.h
    pxtnWoice.h
    pxtoneNoise.h
)

# Winelib can't use a shared pxtone: ld.so would load it behind Wine's back
if(SHARED_PXTONE AND NOT WINELIB)
    set(PROJECT_SCOPE SHARED)
else()
    set(PROJECT_SCOPE STATIC)
endif()
add_pxtone_library(${PROJECT_NAME} ${PROJECT_SCOPE} "${PROJECT_SOURCES};${PROJECT_HEADERS}" ${VORBISFILE_LIBRARY})

if(VORBISFILE_LIBRARY)
    target_compile_definitions(${PROJECT_NAME} PRIVATE pxINCLUDE_OGGVORBIS)
endif()

# Export
# ---------------

add_library(pxtone::pxtone ALIAS ${PROJECT_NAME})
