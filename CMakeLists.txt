# Meta
cmake_minimum_required(VERSION 3.29)
include(CMakeDependentOption)

set(CMAKE_CXX_STANDARD 11)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Configuration options
# ---------------

option(SHARED_PXTONE "Compile pxtone as a shared library" true)

option(ENABLE_PTCOLLAGE "Build ptCollage" true)
option(ENABLE_PTVOICE "Build ptVoice" true)
option(ENABLE_PTNOISE "Build ptNoise" true)
option(ENABLE_PTPLAYER "Build ptPlayer" true)

cmake_dependent_option(ENABLE_PXWINDOWS "Include pxWindows in source tree" true "ENABLE_PTCOLLAGE OR ENABLE_PTVOICE OR ENABLE_PTNOISE OR ENABLE_PTPLAYER" false)

option(USE_UNICODE "Force Unicode" true)
option(FORCE_64_BIT "Force 64-bit" false)

# Compatibility option for MSYS2
# option(FORCE_LLVM_RC "Try to force use of LLVM-RC for compiling resources. Requires llvm-rc and LLVM lld in the PATH and manual specification of CMAKE_RC_IMPLICIT_INCLUDE_DIRECTORIES (/[sysroot]/include on MinGW)" false)

option(BUNDLED_VORBIS "Use the included libvorbis" false)
option(BUNDLED_PNG "Use the included libpng (and zlib)" false)

# using set() instead of option() because it defaults this to "off"
# this is ignored on multi-config generators; the output paths will be respected because they can't be observed
# TODO: consider using CPack instead and defining build presets
set(BUNDLE_BUILD_DIR "" CACHE STRING "Copy build into a pxtone-compatible structure at this path, relative to CMAKE_SOURCE_DIR (-S). If not provided, all binaries will stay in the build directory & any folders within as dictated by the generator, and will have to be moved manually.")

# Base configuration
# ---------------

# This must be specified before the project is initialized
# For testing whether LLVM-RC continues to work with the project's resources, this option is more convenient for me than using Clang
# if(FORCE_LLVM_RC)
#     find_program (LLVM_RC llvm-rc REQUIRED)
#     set(CMAKE_RC_COMPILER "${LLVM_RC}")
#     # code page (-C) 932 is shift JIS, override with UTF-8 (65001)
#     set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <DEFINES> -I \"${CMAKE_RC_IMPLICIT_INCLUDE_DIRECTORIES}\" -C 65001 ${rc_flags} -FO \"<OBJECT>\" \"<SOURCE>\"")
#     set(CMAKE_LINKER_TYPE LLD)
# endif()

project(pxtone LANGUAGES C CXX)

if(WINELIB)
    # winegcc only understands the glued "-xLANG" spelling; the spaced "-x c++-header" CMake emits
    # for precompiled headers makes it drop the input file ("g++: no input files").
    set(CMAKE_CXX_COMPILE_OPTIONS_CREATE_PCH -xc++-header -include <PCH_HEADER>)
endif()

if(WIN32 OR WINELIB)
    if(WINELIB)
      # CMake only compiles resources by default on actual Windows targets
      enable_language(RC)
      add_compile_definitions(OV_EXCLUDE_STATIC_CALLBACKS _WCTYPE_T_DEFINED)
      include_directories(BEFORE "${CMAKE_SOURCE_DIR}/compat/winelib")
      add_link_options("-mwindows")
    else()
      add_compile_options("-Wl,-subsystem,windows")
    endif()
    add_compile_definitions(WIN_32_LEAN_AND_MEAN pxPLATFORM_windows)
    if(USE_UNICODE)
        # win32 library defs
        add_compile_definitions(UNICODE _UNICODE)
        if(MINGW OR WINELIB)
            # compiler defs
            add_compile_options("-municode")
            add_link_options("-municode") # wine needs for wwinmain
        endif()

        # an .rc to wrap the .manifest is required to work with all compilers
        # the executables will automatically source this via. macro
        set(MANIFEST_RC "${CMAKE_BINARY_DIR}/manifest.rc")
        file(WRITE "${MANIFEST_RC}" "1 24 \"${CMAKE_SOURCE_DIR}/pxCommon/utf8.manifest\"\n")
    endif()
endif()

if(FORCE_64_BIT OR CMAKE_SIZEOF_VOID_P EQUAL 8)
  add_compile_definitions(px64BIT)
endif()

# shared functionality library providing pxStdAfx.h and macros
add_subdirectory(pxCommon)

# External dependencies
# ---------------

function(BUNDLED_LIB_ERR)
  message(FATAL_ERROR "Can't find ${arg1};, install it or enable ${arg2}")
endfunction()

if(BUNDLED_VORBIS)
    add_subdirectory(vorbis)
    set(VORBISFILE_LIBRARY pxtone::vorbis)
else()
    find_package(Vorbis)
    if(Vorbis_FOUND)
        set(VORBISFILE_LIBRARY Vorbis::vorbisfile)
    else()
        BUNDLED_LIB_ERR("libvorbis" "BUNDLED_VORBIS")
    endif()
endif()

if(BUNDLED_PNG AND ENABLE_PXWINDOWS)
    add_subdirectory(zlib)
    add_subdirectory(lpng)
    set(PNG_LIBRARIES pxtone::png)
else()
    find_package(PNG)
    if(PNG_FOUND)
        set(PNG_LIBRARIES PNG::PNG)
    else()
    BUNDLED_LIB_ERR("libpng" "BUNDLED_PNG")
    endif()
endif()

# Internal libraries & includes ("producing" targets)
# ---------------

# pxtone library (sound engine)
add_subdirectory(pxtone)

add_subdirectory(Generic)

if (ENABLE_PXWINDOWS)
    add_subdirectory(ptConfig)
    add_subdirectory(pxtoneTool)
    add_subdirectory(pxtonewin)
    add_subdirectory(pxWindows)
    add_subdirectory(pxWorks) # pxtone::works
    add_subdirectory(uxWorks) # pxtone::works2
    add_subdirectory(pxWrapper)
endif()

# Subproject configurations ("consuming" targets)
# ---------------

if (ENABLE_PTCOLLAGE)
    add_subdirectory(ptCollage)
endif()

if (ENABLE_PTVOICE)
    add_subdirectory(ptVoice)
endif()

if (ENABLE_PTNOISE)
    add_subdirectory(ptNoise)
endif()

if (ENABLE_PTPLAYER)
    add_subdirectory(ptPlayer)
endif()
