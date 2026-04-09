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
if(NOT TARGET OBUParse::static AND NOT TARGET OBUParse::shared)
    find_package(OBUParse REQUIRED)
endif()

# --- Source Files ---
set(SRC_COMMON
    "${CMAKE_CURRENT_SOURCE_DIR}/common/alloc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/bits.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/bytes.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/bytes.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/internal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/list.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/memint.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/multibuf.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/multibuf.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/osdep.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/osdep.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/utils.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/utils.h"
)
set(SRC_CODECS
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/a52.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/a52.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/alac.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/av1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/av1.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/av1_obu.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/av1_obu.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/description.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/description.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/dts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/dts.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/h264.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/h264.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/hevc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/hevc.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/id.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/mp4a.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/mp4a.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/mp4sys.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/mp4sys.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/mp4v.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/nalu.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/nalu.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/opus.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/qt_wfex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/vc1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/vc1.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/wma.c"
)
set(SRC_IMPORTER
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/a52_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/adts_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/als_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/amr_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/dts_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/importer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/importer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/isobm_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/ivf_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/mp3_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/nalu_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/vc1_imp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/importer/wave_imp.c"
)
set(SRC_CORE
    "${CMAKE_CURRENT_SOURCE_DIR}/core/box.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/box.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/box_default.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/box_default.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/box_type.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/box_type.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/chapter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/file.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/file.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fragment.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fragment.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/isom.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/meta.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/print.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/print.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/read.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/read.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/summary.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/timeline.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/timeline.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/write.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/write.h"
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
