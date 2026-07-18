# Standalone wrapper to cross-compile the in-tree libwebrtc for the Wii U
# (PowerPC) in CI. Source/ThirdParty/libwebrtc/CMakeLists.txt has no project()
# of its own -- it is meant to be add_subdirectory'd from the full WebKit build
# -- so this wrapper supplies project()/toolchain context and pulls it in.
#
# This is a bring-up harness: it exists to surface libwebrtc's Wii U porting
# errors (endianness, sockets, platform branches, deps) as a build scoreboard,
# the same way the WKC port library was brought up. It does NOT produce the
# final browser; that link happens elsewhere with full WebCore.

cmake_minimum_required(VERSION 3.16)
project(libwebrtc_wiiu CXX C ASM)

# Path to the vendored libwebrtc (overridable from the CI invocation).
if (NOT DEFINED LIBWEBRTC_DIR)
    set(LIBWEBRTC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../Source/ThirdParty/libwebrtc")
endif ()

# Wii U has no AV1 codec path; keep it off (also avoids the dav1d PAL include
# forwarding, which belongs to the full WebCore build).
if (NOT DEFINED ENABLE_AV1)
    set(ENABLE_AV1 OFF CACHE BOOL "" FORCE)
endif ()

add_subdirectory(${LIBWEBRTC_DIR} libwebrtc-build)
