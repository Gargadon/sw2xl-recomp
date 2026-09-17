# sw2xl-recomp - Samurai Warriors 2: Xtreme Legends for ReXGlue
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(sw2xl_recomp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SW2_PGO "AUTO" CACHE STRING
    "PGO mode: AUTO, OFF, GENERATE, or USE (AUTO uses a matching local profile when present)")
set_property(CACHE SW2_PGO PROPERTY STRINGS AUTO OFF GENERATE USE)
set(SW2_PGO_PROFILE "${CMAKE_CURRENT_SOURCE_DIR}/pgo/sw2.profdata" CACHE FILEPATH
    "Merged Clang instrumentation profile used when SW2_PGO=USE")

# The generated base-game target and the XL module retain these identifiers:
# they are part of the ReXGlue-generated ABI, not this project's public name.
if(NOT DEFINED REXGLUE_HOST_TARGET)
    set(REXGLUE_HOST_TARGET samurai_warriors_2)
endif()
include(generated/rexglue.cmake)

# Git for Windows can check out the cabextract compatibility symlinks in
# libmspack as plain text files when symlink support is unavailable.  Use the
# canonical sources that those links reference, without changing the SDK
# checkout.  A normal symlink-capable checkout keeps the SDK's default setup.
if(REXSDK_DIR AND TARGET mspack)
    set(sw2_mspack_cabextract_dir
        "${REXSDK_DIR}/thirdparty/libmspack/cabextract/mspack")
    set(sw2_mspack_canonical_dir
        "${REXSDK_DIR}/thirdparty/libmspack/libmspack/mspack")
    set(sw2_mspack_link_stub "${sw2_mspack_cabextract_dir}/lzxd.c")

    if(EXISTS "${sw2_mspack_canonical_dir}/lzxd.c" AND EXISTS "${sw2_mspack_link_stub}")
        file(READ "${sw2_mspack_link_stub}" sw2_mspack_link_contents LIMIT 256)
        if(sw2_mspack_link_contents MATCHES
                "^[ \t\r\n]*\\.\\./\\.\\./libmspack/mspack/lzxd\\.c[ \t\r\n]*$")
            # The SDK target contains exactly this one source.  Replacing its
            # source list is necessary because CMake has already created the
            # target by the time this project can inspect the checkout.
            set_property(TARGET mspack PROPERTY SOURCES
                "${sw2_mspack_canonical_dir}/lzxd.c")
            target_include_directories(mspack BEFORE PUBLIC
                "$<BUILD_INTERFACE:${sw2_mspack_canonical_dir}>")
            message(STATUS "Using canonical libmspack sources (SDK symlinks unavailable)")
        endif()
    endif()
endif()

# The source-tree ReXGlue SDK uses SSSE3 byte-shuffle intrinsics in the core,
# XMA audio, and Xenos graphics targets. Apply the required AMD64 feature from
# this project, leaving the SDK checkout unmodified. These concrete targets
# exist only when REXSDK_DIR is used; an installed SDK supplies its own
# compile settings.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
    foreach(sw2_sdk_target IN ITEMS rexcore rexaudio rexgpu-xenos)
        if(TARGET ${sw2_sdk_target})
            target_compile_options(${sw2_sdk_target} PRIVATE -mssse3)
        endif()
    endforeach()
endif()

# Sources
set(SAMURAIWARRIORS2_SOURCES
    src/main.cpp
    src/heap_diagnostics.cpp
    src/event_fix.cpp
    src/input_diagnostics.cpp
    src/dlc_installer.cpp
)

if(WIN32)
    add_executable(samurai_warriors_2 WIN32 ${SAMURAIWARRIORS2_SOURCES})
else()
    add_executable(samurai_warriors_2 ${SAMURAIWARRIORS2_SOURCES})
endif()

rexglue_setup_target(samurai_warriors_2 GPU_PLUGINS xenos)

# rex::runtime uses ImGui privately, but the host target also compiles runtime
# UI sources and includes rex/ui/style.h.  Propagate ImGui's public include
# directory to that compilation without changing the SDK's CMake files.
if(TARGET imgui)
    target_link_libraries(samurai_warriors_2 PRIVATE imgui)
endif()

# The installed SDK toolchain leaves CMAKE_CXX_FLAGS_RELEASE empty. Generated
# guest code is prohibitively slow without compiler optimization, affecting
# video playback, simulation speed, and how often the game polls input.
foreach(sw2_target IN ITEMS
        samurai_warriors_2
        samurai_warriors_2_recomp
        samurai_warriors_2_SW2XL_US)
    if(TARGET ${sw2_target})
        target_compile_options(${sw2_target} PRIVATE
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Release>:-DNDEBUG>
            $<$<CONFIG:Release>:-flto=thin>
        )
        if(SW2_PGO STREQUAL "GENERATE")
            target_compile_options(${sw2_target} PRIVATE -fprofile-instr-generate)
            target_link_options(${sw2_target} PRIVATE -fprofile-instr-generate)
        elseif(SW2_PGO STREQUAL "AUTO" OR SW2_PGO STREQUAL "USE")
            if(NOT EXISTS "${SW2_PGO_PROFILE}")
                if(SW2_PGO STREQUAL "USE")
                    message(FATAL_ERROR "PGO profile not found: ${SW2_PGO_PROFILE}")
                endif()
                message(STATUS "PGO profile not found; building without PGO: ${SW2_PGO_PROFILE}")
            else()
                message(STATUS "Using PGO profile: ${SW2_PGO_PROFILE}")
                target_compile_options(${sw2_target} PRIVATE
                    "-fprofile-instr-use=${SW2_PGO_PROFILE}")
                target_link_options(${sw2_target} PRIVATE
                    "-fprofile-instr-use=${SW2_PGO_PROFILE}")
            endif()
        elseif(NOT SW2_PGO STREQUAL "OFF")
            message(FATAL_ERROR "SW2_PGO must be AUTO, OFF, GENERATE, or USE")
        endif()
    endif()
endforeach()

foreach(sw2_link_target IN ITEMS samurai_warriors_2 samurai_warriors_2_SW2XL_US)
    if(TARGET ${sw2_link_target})
        target_link_options(${sw2_link_target} PRIVATE
            $<$<CONFIG:Release>:-flto=thin>)
        if(WIN32)
            target_link_options(${sw2_link_target} PRIVATE
                $<$<CONFIG:Release>:-Wl,/OPT:REF>
                $<$<CONFIG:Release>:-Wl,/OPT:ICF>)
        endif()
    endif()
endforeach()

if(TARGET samurai_warriors_2_SW2XL_US)
    target_sources(samurai_warriors_2_SW2XL_US PRIVATE src/event_fix.cpp)
    if(WIN32)
        add_custom_target(sw2_patch_xl_codegen
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass
                -File "${CMAKE_CURRENT_SOURCE_DIR}/patch-xl-codegen.ps1"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/sw2xl_us/samurai_warriors_2_init.cpp"
            VERBATIM)
    else()
        add_custom_target(sw2_patch_xl_codegen
            COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/patch-xl-codegen.sh"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/sw2xl_us/samurai_warriors_2_init.cpp"
            VERBATIM)
    endif()
    # Codegen rewrites module_registry.cpp. Ensure this project-side patch
    # always runs after it, rather than racing its generated outputs.
    add_dependencies(sw2_patch_xl_codegen samurai_warriors_2_codegen)
    add_dependencies(samurai_warriors_2_SW2XL_US sw2_patch_xl_codegen)
    add_dependencies(samurai_warriors_2 sw2_patch_xl_codegen)
endif()

# Retain native function addresses for diagnosing Windows crash dumps.
if(WIN32)
    target_link_options(samurai_warriors_2 PRIVATE "-Wl,/MAP")
    if(TARGET samurai_warriors_2_SW2XL_US)
        target_link_options(samurai_warriors_2_SW2XL_US PRIVATE
            "-Wl,/MAP:${CMAKE_CURRENT_BINARY_DIR}/samurai_warriors_2_SW2XL_US.map")
    endif()
endif()
