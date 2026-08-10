# CMakeLists.txt for EIC-adapted PYTHIA6, PYTHIA-RAD-CORR.
#
# To build using cmake, create a build directory, navigate
# to it and run cmake. e.g. assuming we start in the PYTHIA-RAD-CORR
# source directory:
#  mkdir build
#  cd build
#  cmake .. (or whatever the path to the source directory is)
#
# You can specify an install directory via
#  -DCMAKE_INSTALL_PREFIX=<path>
#
# then do
#  make install
# to install the headers and libraries under that location.
# There is no "make uninstall" but (on Unix-like systems)
# you can do
#  xargs rm < install_manifest.txt
# from the cmake build directory.
#
# Run everything from this single file because the sources
# from each subdirectory have to be combined in a single
# shared library, rather than being one library per directory.

cmake_minimum_required(VERSION 3.7)

# Build or not to build tests
option(BUILD_CPP "Build c++ wrappers" OFF)

project(PYTHIA-RAD-CORR Fortran C CXX)

SET(VERSION 1.0.0)

# Add this project's module path to the default path.
# Needed to locate this project's FindROOT.cmake.
set(
   CMAKE_MODULE_PATH
   ${CMAKE_MODULE_PATH}
   ${CMAKE_SOURCE_DIR}/cmake/Modules/
)

# This module sets up ROOT information
# It defines:
# ROOT_FOUND             If the ROOT is found
# ROOT_INCLUDE_DIR       PATH to the include directory
# ROOT_INCLUDE_DIRS      PATH to the include directories (not cached)
# ROOT_LIBRARIES         Most common libraries
# ROOT_<name>_LIBRARY    Full path to the library <name>
# ROOT_LIBRARY_DIR       PATH to the library directory
# ROOT_ETC_DIR           PATH to the etc directory
# ROOT_DEFINITIONS       Compiler definitions
# ROOT_CXX_FLAGS         Compiler flags to used by client packages
# ROOT_C_FLAGS           Compiler flags to used by client packages
# ROOT_EXE_LINKER_FLAGS  Linker flags to used by client packages
#
# Updated by K. Smith (ksmith37@nd.edu) to properly handle
#  dependencies in ROOT_GENERATE_DICTIONARY

message (STATUS "Looking for ROOT...")
find_package(ROOT)

if ( NOT ROOT_FOUND )
  message (FATAL_ERROR "Couldn't find ROOT")
else()
  Message(STATUS "ROOT Include dir : " ${ROOT_INCLUDE_DIRS})
  Message(STATUS "ROOT Library Path : " ${ROOT_LIBRARY_DIR})
  include_directories(${ROOT_INCLUDE_DIRS})
endif()

# ROOT6 needs C++11 standard
string(SUBSTRING ${ROOT_VERSION} 0 1 ROOT_MAJOR)

IF(${ROOT_MAJOR} GREATER 5)
  IF(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
  ENDIF()
ENDIF()

# - Try to find CERNLIB
# Once done this will define
#  CERNLIB_FOUND - system has CERNLIB
#  CERNLIB_LIB_PATH - The path to CERNLIB
# The cmake Module claims other variables are set, that is not the case

find_package(CERNLIB)
if ( NOT CERNLIB_FOUND )
  find_package(nanocernlib)
  if(NOT ${nanocernlib_LIBRARIES})
    message (FATAL_ERROR "Couldn't find CERNLIB or NANOCERNLIB")
  else()
    message(STATUS "Using nanocernlib at ${NANOCERNLIB_INCLUDE_DIR}")
    set (CERNLIB_LIBRARIES ${NANOCERNLIB_LIBRARIES})
  endif()
else()
  set (CERNLIB_LIBRARIES "-L${CERNLIB_LIBRARY_DIR} -lpacklib -lmathlib -lkernlib -lpdflib804")
endif()
message (STATUS "CERNLIB flags:" ${CERNLIB_LIBRARIES} )


## drivers use pythia-radcorr, but for main library we'll keep the
## second, explicit include path to keep the source clean and simple
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/pythia-radcorr)

set(CMAKE_Fortran_FLAGS "-fno-second-underscore -fPIC")
# newer fortran compilers cause problems
include(CheckFortranCompilerFlag)
check_fortran_compiler_flag("-std=legacy" _std_legacy)
if(_std_legacy)
  set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -std=legacy")
endif()
check_fortran_compiler_flag("-fallow-argument-mismatch" _fallow_argument_mismatch)
if(_fallow_argument_mismatch)
  set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fallow-argument-mismatch")
endif()

# Pythia sources
FILE(GLOB PYTHIA_SOURCE src/pythia/*.f)

# Additional sources for radiative corrections
FILE(GLOB RADCORR_SOURCE src/radcorr/*.f)


# Organize into "object" library to avoid double compilation
add_library(
  pythia6-eicmod-objects
  OBJECT
  ${PYTHIA_SOURCE}
  ${RADCORR_SOURCE}
)

# Build main static library
add_library(
  pythia6-eicmod
  STATIC
  $<TARGET_OBJECTS:pythia6-eicmod-objects>
)

# Build original fortran driver
# IMPORTANT: Linking here explicitly egainst LHAPDF5 assuming LHAPDF5
# is a defined environment variable. One could and maybe should use
# a cmake module and/or command line arguments, but the choice between
# LHAPDF5 and 6 makes this finicky.
# Side note: This driver uses a very common fortran extension that
# produces warnings in gfortran
add_executable(pythia6-eic src/drivers/pythia6-eic.f)
# target_link_libraries(pythia6-eic pythia6-eicmod -L$ENV{LHAPDF5} -L$ENV{LHAPDF5}/lib -lLHAPDF ${CERNLIB_LIBRARIES} )

target_link_libraries(pythia6-eic pythia6-eicmod -L$ENV{LHAPDF} -L$ENV{LHAPDF}/lib -lLHAPDF ${CERNLIB_LIBRARIES} )

# Build ROOT interface
# Additional sources for radiative corrections
# ROOT will not pick up this library if we change the name
# So be carefull, if you install another pythia this may get overwritten
add_library(
  Pythia6
  SHARED
  $<TARGET_OBJECTS:pythia6-eicmod-objects>
  src/tpythia6/main.c
  src/tpythia6/pythia6_common_address.c
  src/tpythia6/tpythia6_called_from_cc.F
  )

# target_link_libraries(Pythia6 -L${CERNLIB_LIBRARY_DIR} -lpacklib -lmathlib  -lpacklib -lmathlib -lkernlib -L$ENV{LHAPDF5} -L$ENV{LHAPDF5}/lib -lLHAPDF )
# target_link_libraries(Pythia6 -L$ENV{LHAPDF5} -L$ENV{LHAPDF5}/lib -lLHAPDF ${CERNLIB_LIBRARIES} )
target_link_libraries(Pythia6 -L$ENV{LHAPDF} -L$ENV{LHAPDF}/lib -lLHAPDF ${CERNLIB_LIBRARIES} )
## Note: here, too, we're linking against LHAPDF directly
## That is not strictly necessary.
## One could also use
# target_link_libraries(Pythia6 pythia6-eicmod -L${CERNLIB_LIBRARY_DIR} -lpacklib -lmathlib  -lpacklib -lmathlib -lkernlib )
## and link to LHAPDF only for executables or even at runtime.
## In that case, on a Mac adding '-undefined dynamic_lookup' as a linker option is necessary
## (May additionally need '-flat_namespace' ?)

# # Build c++ drivers
# ## Same comments about dynamic libraries applies
if(BUILD_CPP)
  message (STATUS " -- Building c++ wrappers - requires ROOT")

  add_executable(pythiaMain src/drivers/pythiaMain.cpp)
  # target_link_libraries(pythiaMain ${ROOT_LIBRARIES} -L${ROOT_LIBRARY_DIR} -lEG -lEGPythia6 Pythia6-eicmod pythia6-eicmod -L$ENV{LHAPDF5} -L$ENV{LHAPDF5}/lib -lLHAPDF ${CERNLIB_LIBRARIES})
  target_link_libraries(pythiaMain ${ROOT_LIBRARIES} -L${ROOT_LIBRARY_DIR} -lEG -lEGPythia6 Pythia6-eicmod pythia6-eicmod -L$ENV{LHAPDF} -L$ENV{LHAPDF}/lib -lLHAPDF ${CERNLIB_LIBRARIES})
  
  add_executable(UsingCardPythiaMain src/drivers/UsingCardPythiaMain.cpp)
  # target_link_libraries(UsingCardPythiaMain ${ROOT_LIBRARIES} -L${ROOT_LIBRARY_DIR} -lEG -lEGPythia6-eicmod Pythia6 pythia6-eicmod -L$ENV{LHAPDF5} -L$ENV{LHAPDF5}/lib -lLHAPDF ${CERNLIB_LIBRARIES} )
  target_link_libraries(UsingCardPythiaMain ${ROOT_LIBRARIES} -L${ROOT_LIBRARY_DIR} -lEG -lEGPythia6-eicmod Pythia6 pythia6-eicmod -L$ENV{LHAPDF} -L$ENV{LHAPDF}/lib -lLHAPDF ${CERNLIB_LIBRARIES} )
endif()

## Install headers, libraries, executables
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/pythia-radcorr DESTINATION include )
INSTALL(TARGETS pythia6-eicmod Pythia6 DESTINATION lib )
INSTALL(TARGETS pythia6-eic DESTINATION bin )
## For backward compatibility, symlink to the new executable
INSTALL(
  CODE "MESSAGE(\"-- Creating symbolic link ${CMAKE_INSTALL_PREFIX}/bin/pythiaeRHIC -> ${CMAKE_INSTALL_PREFIX}/bin/pythia6-eic\")"
  CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/bin/pythia6-eic ${CMAKE_INSTALL_PREFIX}/bin/pythiaeRHIC )"
  )

if(BUILD_CPP)
  INSTALL(TARGETS UsingCardPythiaMain DESTINATION bin )
  INSTALL(TARGETS pythiaMain DESTINATION bin )
endif()

