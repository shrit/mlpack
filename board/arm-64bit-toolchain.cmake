## This file handles cross-compilation configurations for aarch64,
## known as arm64. The objective of this file is to find and assign
## cross-compiler and the entire toolchain.
## It works best with buildroot toolchain, when using it the user
## needs to set the: TOOLCHAIN_PREFIX and CMAKE_SYSROOT from the
## command line.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ARM)

if(MINGW OR CYGWIN OR WIN32)
    set(UTIL_SEARCH_CMD where)
elseif(UNIX OR APPLE)
    set(UTIL_SEARCH_CMD which)
endif()

if(DEFINED TOOLCHAIN_PREFIX AND DEFINED CMAKE_SYSROOT)
  message(STATUS "TOOLCHAIN_PREFIX is defined as: ${TOOLCHAIN_PREFIX}")
  message(STATUS "CMAKE_SYSROOT is defined as: ${CMAKE_SYSROOT}")
else ()
  ## standard gcc toolchain
  set(TOOLCHAIN_PREFIX aarch64-linux-gnu-)
  ## There is no need to specify the CMAKE_SYSROOT if you are using the 
  ## standard toolchain. If you download your own toolchain you have to specify
  ## the path for sysroot as follows:
  ## set(CMAKE_SYSROOT /PathToToolchain/aarch64-buildroot-linux-gnu/sysroot)
  ## or it can be specified from command-line.
  set(CMAKE_SYSROOT /usr/aarch64-linux-gnu/ CACHE STRING "Enter path for sysroot")
endif()
## In some distribution, a dynamic link for aarch64-linux-gnu-gcc may not be
## found or created, instead it might be labeled with the version at the end
## For instance: aarch64-linux-gnu-gcc-5
## Therefore, if dynamic link exists, you do not have to specify the version
set(VERSION_NUMBER "" CACHE STRING "Enter the version number of the compiler")

# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_AR "${TOOLCHAIN_PREFIX}gcc-ar${VERSION_NUMBER}" CACHE FILEPATH "" FORCE)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc${VERSION_NUMBER})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++${VERSION_NUMBER})
set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}ld${VERSION_NUMBER})
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH  true)
set(CMAKE_FORTRAN_COMPILER ${TOOLCHAIN_PREFIX}gfortran)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy${VERSION_NUMBER} CACHE INTERNAL "objcopy tool")
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_PREFIX}size${VERSION_NUMBER} CACHE INTERNAL "size tool")

## Here are the standard ROOT_PATH if you are using the standard toolchain
## if you are using a different toolchain you have to specify that too
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --sysroot=${CMAKE_SYSROOT}" CACHE INTERNAL "" FORCE)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
