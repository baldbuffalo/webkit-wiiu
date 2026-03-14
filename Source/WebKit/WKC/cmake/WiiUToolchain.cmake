cmake_minimum_required(VERSION 3.16)

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_PROCESSOR  powerpc)

if(NOT DEFINED ENV{DEVKITPRO})
    message(FATAL_ERROR "DEVKITPRO environment variable not set")
endif()
set(DEVKITPRO  $ENV{DEVKITPRO})
set(DEVKITPPC  ${DEVKITPRO}/devkitPPC)
set(WUT_ROOT   ${DEVKITPRO}/wut)
set(PORTLIBS   ${DEVKITPRO}/portlibs/wiiu)
set(PORTLIBS_PPC ${DEVKITPRO}/portlibs/ppc)

set(CMAKE_C_COMPILER   ${DEVKITPPC}/bin/powerpc-eabi-gcc   CACHE PATH "")
set(CMAKE_CXX_COMPILER ${DEVKITPPC}/bin/powerpc-eabi-g++   CACHE PATH "")
set(CMAKE_AR           ${DEVKITPPC}/bin/powerpc-eabi-ar     CACHE PATH "")
set(CMAKE_RANLIB       ${DEVKITPPC}/bin/powerpc-eabi-ranlib CACHE PATH "")
set(CMAKE_STRIP        ${DEVKITPPC}/bin/powerpc-eabi-strip  CACHE PATH "")

set(WIIU_FLAGS "-mcpu=750 -meabi -mhard-float -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS_INIT   "${WIIU_FLAGS}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_INIT "${WIIU_FLAGS} -fno-exceptions -fno-rtti -std=c++20" CACHE STRING "")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()
set(CMAKE_C_FLAGS_RELEASE   "-O2" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING "")

include_directories(
    ${WUT_ROOT}/include
    ${PORTLIBS}/include
    ${PORTLIBS_PPC}/include
    ${PORTLIBS_PPC}/include/freetype2
)

link_directories(
    ${WUT_ROOT}/lib
    ${PORTLIBS}/lib
    ${PORTLIBS_PPC}/lib
)

set(CMAKE_FIND_ROOT_PATH ${DEVKITPPC} ${WUT_ROOT} ${PORTLIBS} ${PORTLIBS_PPC})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_definitions(-DWKC_CUSTOMER_WIIU)

set(DEVKITPRO  ${DEVKITPRO} CACHE PATH "devkitPro root")
set(WUT_ROOT   ${WUT_ROOT}  CACHE PATH "WUT root")
set(PORTLIBS   ${PORTLIBS}  CACHE PATH "Wii U portlibs")
