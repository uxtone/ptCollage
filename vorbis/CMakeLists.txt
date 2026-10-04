# Ad-hoc CMake module exposing all libogg and libvorbis sources & headers from in-tree libogg and libvorbis
# This is the only bundled library where its .vcxproj exists in its own directory, and we will match

project(pxtone-vorbis LANGUAGES C)

function(external_sources dst prefix)
   set(lv "")
   foreach(f ${ARGN})
      list(APPEND lv "${prefix}/${f}")
   endforeach(f)
   set(${dst} "${lv}" PARENT_SCOPE)
endfunction()

set(OGG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/libogg")
set(VORBIS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/libvorbis")

external_sources(OGG_SOURCES "${OGG_SOURCE_DIR}/src"
	bitwise.c
	framing.c
)

external_sources(VORBIS_SOURCES "${VORBIS_SOURCE_DIR}/lib"
	analysis.c
	bitrate.c
	block.c
	codebook.c
	envelope.c
	floor0.c
	floor1.c
	info.c
	lookup.c
	lpc.c
	lsp.c
	mapping0.c
	mdct.c
	psy.c
	registry.c
	res0.c
	sharedbook.c
	smallft.c
	synthesis.c
	vorbisenc.c
	vorbisfile.c
	window.c
)

list(APPEND LIBRARY_SOURCES ${OGG_SOURCES})
list(APPEND LIBRARY_SOURCES ${VORBIS_SOURCES})

list(APPEND LIBRARY_DIRS "${VORBIS_SOURCE_DIR}/lib")
list(APPEND LIBRARY_DIRS "${VORBIS_SOURCE_DIR}/include")
list(APPEND LIBRARY_DIRS "${OGG_SOURCE_DIR}/include")

# <ogg/os_types.h> includes <ogg/config_types.h> on every platform it has no special case for.
# libogg's configure normally writes it; these are the values it finds on any C99 system.
set(INCLUDE_INTTYPES_H 1)
set(INCLUDE_STDINT_H 1)
set(INCLUDE_SYS_TYPES_H 1)
set(SIZE16 int16_t)
set(USIZE16 uint16_t)
set(SIZE32 int32_t)
set(USIZE32 uint32_t)
set(SIZE64 int64_t)
configure_file("${OGG_SOURCE_DIR}/include/ogg/config_types.h.in" "${CMAKE_CURRENT_BINARY_DIR}/include/ogg/config_types.h" @ONLY)
list(APPEND LIBRARY_DIRS "${CMAKE_CURRENT_BINARY_DIR}/include")

add_bundled_library(${PROJECT_NAME} SHARED "${LIBRARY_SOURCES}" "${LIBRARY_DIRS}")

# Export
# ---------------

add_library(pxtone::vorbis ALIAS ${PROJECT_NAME})
