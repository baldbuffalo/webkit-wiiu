cmake_minimum_required(VERSION 3.16)
project(WebKitWiiU CXX C)

set(WKC_DIR     ${CMAKE_SOURCE_DIR})
set(WEBCORE_DIR ${CMAKE_SOURCE_DIR}/../../WebCore)
set(WTF_DIR     ${CMAKE_SOURCE_DIR}/../../WTF)
set(JSC_DIR     ${CMAKE_SOURCE_DIR}/../../JavaScriptCore)
set(BMALLOC_DIR ${CMAKE_SOURCE_DIR}/../../bmalloc)
get_filename_component(SOURCE_DIR "${CMAKE_SOURCE_DIR}/../.." ABSOLUTE)

include_directories(
    ${WKC_DIR}/stubs
    ${SOURCE_DIR}
    ${JSC_DIR}
    ${JSC_DIR}/runtime
    ${WEBCORE_DIR}
    ${WEBCORE_DIR}/WebCore
    ${WEBCORE_DIR}/PAL
    ${WEBCORE_DIR}/PAL/pal
    ${WEBCORE_DIR}/bindings
    ${WEBCORE_DIR}/bindings/js
    ${WEBCORE_DIR}/dom
    ${WEBCORE_DIR}/editing
    ${WEBCORE_DIR}/fileapi
    ${WEBCORE_DIR}/html
    ${WEBCORE_DIR}/html/shadow
    ${WEBCORE_DIR}/loader
    ${WEBCORE_DIR}/loader/cache
    ${WEBCORE_DIR}/page
    ${WEBCORE_DIR}/platform
    ${WEBCORE_DIR}/platform/WKC
    ${WEBCORE_DIR}/platform/graphics
    ${WEBCORE_DIR}/platform/graphics/WKC
    ${WEBCORE_DIR}/platform/network
    ${WEBCORE_DIR}/platform/network/WKC
    ${WEBCORE_DIR}/platform/audio/WKC
    ${WEBCORE_DIR}/platform/text/WKC
    ${WEBCORE_DIR}/rendering
    ${WEBCORE_DIR}/style
    ${WEBCORE_DIR}/svg
    ${WTF_DIR}
    ${WTF_DIR}/wtf
    ${WTF_DIR}/wtf/WKC
    ${WTF_DIR}/wtf/text
    ${BMALLOC_DIR}
    /tmp/icu-headers
    ${WKC_DIR}
    ${WKC_DIR}/include
    ${WKC_DIR}/include/wkc
    ${WKC_DIR}/webkit
    ${WKC_DIR}/WebCoreSupport
    ${WKC_DIR}/helpers
    ${WKC_DIR}/helpers/privates
    ${WKC_DIR}/accessor
    ${WKC_DIR}/accessor/privates
)

add_definitions(
    -DENABLE_WKC=1
    -DWTF_PLATFORM_WKC=1
    -DUSE_CURL=1
    -DUSE_FREETYPE=1
    -DWTF_USE_BMALLOC=0
    -DUSE_SYSTEM_MALLOC=1
    -DUSE_PTHREADS=1
    -DENABLE_JAVASCRIPT_DEBUGGER=0
    -DENABLE_WEBGL=0
    -DENABLE_WEB_AUDIO=0
    -DENABLE_MEDIA_STREAM=0
    -DENABLE_WEB_RTC=0
    -DENABLE_NOTIFICATIONS=0
    -DENABLE_FULLSCREEN_API=0
    -DENABLE_INDEXED_DATABASE=0
    -DENABLE_SVG=1
    -DENABLE_CSS_ANIMATIONS=1
    -DENABLE_XSLT=0
    "-DWEBCORE_EXPORT="
    "-DPAL_EXPORT="
    "-DWTF_EXPORT_PRIVATE="
    "-DJSEXPORT="
)

add_compile_options(-include ${WKC_DIR}/stubs/WKCPlatformStubs.h)

file(GLOB_RECURSE WTF_WKC_SOURCES          ${WTF_DIR}/wtf/WKC/*.cpp)
file(GLOB_RECURSE WEBCORE_PLATFORM_SOURCES ${WEBCORE_DIR}/platform/WKC/*.cpp
                                           ${WEBCORE_DIR}/platform/network/WKC/*.cpp
                                           ${WEBCORE_DIR}/platform/graphics/WKC/*.cpp
                                           ${WEBCORE_DIR}/platform/text/WKC/*.cpp)
file(GLOB_RECURSE WKC_WEBKIT_SOURCES       ${WKC_DIR}/webkit/*.cpp
                                           ${WKC_DIR}/WebCoreSupport/*.cpp
                                           ${WKC_DIR}/helpers/*.cpp
                                           ${WKC_DIR}/accessor/*.cpp)

list(REMOVE_ITEM WEBCORE_PLATFORM_SOURCES
    ${WEBCORE_DIR}/platform/WKC/ClipboardWKC.cpp
)

add_library(WebKitWKC STATIC
    ${WTF_WKC_SOURCES}
    ${WEBCORE_PLATFORM_SOURCES}
    ${WKC_WEBKIT_SOURCES}
)

target_include_directories(WebKitWKC PUBLIC
    ${WKC_DIR}/include
    ${WKC_DIR}/include/wkc
    ${WKC_DIR}/webkit
)

install(TARGETS WebKitWKC DESTINATION lib)
install(DIRECTORY ${WKC_DIR}/include/ DESTINATION include/wkc)
