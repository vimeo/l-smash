cmake_minimum_required(VERSION 3.15)

# Extract version from lsmash.h
file(STRINGS "lsmash.h" LSMASH_VERSION_MAJOR REGEX "^#define LSMASH_VERSION_MAJOR +([0-9]+)")
string(REGEX REPLACE "[^0-9]" "" LSMASH_VERSION_MAJOR "${LSMASH_VERSION_MAJOR}")
file(STRINGS "lsmash.h" LSMASH_VERSION_MINOR REGEX "^#define LSMASH_VERSION_MINOR +([0-9]+)")
string(REGEX REPLACE "[^0-9]" "" LSMASH_VERSION_MINOR "${LSMASH_VERSION_MINOR}")
file(STRINGS "lsmash.h" LSMASH_VERSION_MICRO REGEX "^#define LSMASH_VERSION_MICRO +([0-9]+)")
string(REGEX REPLACE "[^0-9]" "" LSMASH_VERSION_MICRO "${LSMASH_VERSION_MICRO}")
set(LSMASH_VERSION "${LSMASH_VERSION_MAJOR}.${LSMASH_VERSION_MINOR}.${LSMASH_VERSION_MICRO}")

project(LSMASH VERSION ${LSMASH_VERSION} LANGUAGES C)

# --- Build Options ---
option(BUILD_SHARED_LIBS "Build the shared library" ON)
option(BUILD_STATIC_LIBS "Build the static library" ON)
option(LSMASH_BUILD_TOOLS "Build the command-line tools" ON)

if(NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
    message(FATAL_ERROR "You must build at least one library type.")
endif()

# --- Generate config.h ---
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE LSMASH_REV
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --always
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE LSMASH_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()
if(NOT LSMASH_REV)
    set(LSMASH_REV "0")
endif()
if(NOT LSMASH_GIT_HASH)
    set(LSMASH_GIT_HASH "unknown")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)
# --- Dependencies ---
# L-SMASH requires obuparse. We find it here.
find_package(OBUParse REQUIRED)

# --- Source Files ---
set(SRC_COMMON
    common/alloc.c    common/bits.c    common/bytes.c
    common/list.c     common/multibuf.c common/osdep.c    common/utils.c
)
set(SRC_CODECS
    codecs/a52.c         codecs/alac.c        codecs/av1.c
    codecs/av1_obu.c     codecs/description.c codecs/dts.c
    codecs/h264.c        codecs/hevc.c        codecs/id.c
    codecs/mp4sys.c      codecs/mp4a.c        codecs/mp4v.c
    codecs/nalu.c        codecs/opus.c        codecs/qt_wfex.c
    codecs/vc1.c         codecs/wma.c
)
set(SRC_IMPORTER
    importer/a52_imp.c   importer/adts_imp.c  importer/als_imp.c
    importer/amr_imp.c   importer/dts_imp.c   importer/importer.c
    importer/isobm_imp.c importer/ivf_imp.c   importer/mp3_imp.c
    importer/nalu_imp.c  importer/vc1_imp.c   importer/wave_imp.c
)
set(SRC_CORE
    core/box.c         core/box_default.c core/box_type.c
    core/chapter.c     core/file.c        core/fragment.c
    core/isom.c        core/meta.c        core/print.c
    core/read.c        core/summary.c     core/timeline.c
    core/write.c
)
set(LSMASH_SOURCES
    ${SRC_COMMON} ${SRC_CODECS} ${SRC_IMPORTER} ${SRC_CORE}
)

# --- Library Targets ---

if(BUILD_SHARED_LIBS)
    add_library(lsmash_shared SHARED ${LSMASH_SOURCES})
    target_compile_definitions(lsmash_shared PRIVATE LSMASH_EXPORTS)
    set_target_properties(lsmash_shared PROPERTIES
        OUTPUT_NAME "lsmash"
        VERSION ${LSMASH_VERSION}
        SOVERSION ${LSMASH_VERSION_MAJOR}
        EXPORT_NAME "shared"
    )
    add_library(LSMASH::shared ALIAS lsmash_shared)
endif()

if(BUILD_STATIC_LIBS)
    add_library(lsmash_static STATIC ${LSMASH_SOURCES})
    # On MSVC, rename the static lib to avoid collision with the shared import lib.
    if(MSVC)
        set_target_properties(lsmash_static PROPERTIES OUTPUT_NAME "lsmashs")
    else()
        set_target_properties(lsmash_static PROPERTIES OUTPUT_NAME "lsmash")
    endif()
    set_target_properties(lsmash_static PROPERTIES EXPORT_NAME "static")
    add_library(LSMASH::static ALIAS lsmash_static)
endif()

# --- Common Properties for Both Library Targets ---
set(BUILT_LIB_TARGETS "")
if(TARGET lsmash_shared)
    list(APPEND BUILT_LIB_TARGETS lsmash_shared)
endif()
if(TARGET lsmash_static)
    list(APPEND BUILT_LIB_TARGETS lsmash_static)
endif()

# Determine which OBUParse target to link against.
# Prefer shared if it exists, otherwise use static. This mimics standard Unix behavior.
if(TARGET OBUParse::shared)
    set(OBUParse_LINK_TARGET OBUParse::shared)
else()
    set(OBUParse_LINK_TARGET OBUParse::static)
endif()
message(STATUS "L-SMASH will link against ${OBUParse_LINK_TARGET}")

foreach(target IN LISTS BUILT_LIB_TARGETS)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    # Link against the chosen OBUParse target
    target_link_libraries(${target} PUBLIC ${OBUParse_LINK_TARGET})
    if(UNIX)
        target_link_libraries(${target} PUBLIC m)
    endif()
endforeach()


# --- Tools ---
set(BUILT_TOOLS_TARGETS "")
if(LSMASH_BUILD_TOOLS)
    set(LSMASH_TOOLS_LIST muxer remuxer boxdumper timelineeditor)
    foreach(tool IN LISTS LSMASH_TOOLS_LIST)
        add_executable(${tool}
            cli/${tool}.c
            cli/cli.c
        )
        target_include_directories(${tool} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
        )
        if(TARGET lsmash_shared)
            target_link_libraries(${tool} PRIVATE lsmash_shared)
        else()
            target_link_libraries(${tool} PRIVATE lsmash_static)
        endif()
        list(APPEND BUILT_TOOLS_TARGETS ${tool})
    endforeach()
endif()

# --- RPATH Handling (Build Tree and Install Tree) ---
if(UNIX AND BUILT_TOOLS_TARGETS)
    # This handles the build tree, allowing tools to be run from the build directory.
    set_property(TARGET ${BUILT_TOOLS_TARGETS} PROPERTY
        BUILD_RPATH "${CMAKE_BINARY_DIR}"
    )

    # This handles the install tree. It sets the RPATH on the *installed* executables.
    set_property(TARGET ${BUILT_TOOLS_TARGETS} PROPERTY
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

# --- Installation ---
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(FILES lsmash.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

set(INSTALL_TARGETS "")
if(TARGET lsmash_static)
    list(APPEND INSTALL_TARGETS lsmash_static)
endif()
if(TARGET lsmash_shared)
    list(APPEND INSTALL_TARGETS lsmash_shared)
endif()
if(BUILT_TOOLS_TARGETS)
    list(APPEND INSTALL_TARGETS ${BUILT_TOOLS_TARGETS})
endif()

install(TARGETS ${INSTALL_TARGETS}
    EXPORT LSMASHTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# Generate and install CMake package files
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lsmash-config-version.cmake"
    VERSION ${LSMASH_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
    "lsmash-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lsmash-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/lsmash"
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lsmash-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/lsmash-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/lsmash"
)
install(
    EXPORT LSMASHTargets
    FILE LSMASHTargets.cmake
    NAMESPACE LSMASH::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/lsmash"
)

# --- Uninstall Target ---
# (This part is identical to the obuparse solution)
if(NOT TARGET uninstall)
  configure_file(
    "cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
  )
  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    COMMENT "Uninstalling the project..."
  )
endif()
