# Ad-hoc CMake module exposing all zlib sources & headers from in-tree

project(pxtone-zlib LANGUAGES C)

set(LIBRARY_SOURCES
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    trees.c
    uncompr.c
    zutil.c
)

add_bundled_library(${PROJECT_NAME} SHARED "${LIBRARY_SOURCES}" "${CMAKE_CURRENT_SOURCE_DIR}")

# Export
# ---------------

add_library(pxtone::zlib ALIAS ${PROJECT_NAME})

if(WINELIB)
    # gz* file I/O assumes the MSVC runtime on _WIN32; libpng doesn't use it
    get_target_property(_zsrc pxtone-zlib SOURCES)
    list(FILTER _zsrc EXCLUDE REGEX "gz[a-z]+\\.c$")
    set_target_properties(pxtone-zlib PROPERTIES SOURCES "${_zsrc}")
    target_compile_definitions(pxtone-zlib PRIVATE Z_SOLO)
endif()
