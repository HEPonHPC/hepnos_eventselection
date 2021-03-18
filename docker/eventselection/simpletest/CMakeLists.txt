project                     (simpletest)
cmake_minimum_required      (VERSION 3.9)


# C++11
set                         (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# OSX flags
if                          (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions           (-DMAC_OSX)
  set                       (CMAKE_MACOSX_RPATH on)

    # --- following RPATH settings are for Sierra w/ Clang, hopefully they don't hurt other versions
    # ref: https://cmake.org/Wiki/CMake_RPATH_handling
    # use, i.e. don't skip, the full RPATH for the build tree
    set                     (CMAKE_SKIP_BUILD_RPATH            false)
    # when building, don't use the install RPATH already (but later on when installing)
    set                     (CMAKE_BUILD_WITH_INSTALL_RPATH    false)
    # set RPATH to install path
    set                     (CMAKE_INSTALL_RPATH               "${CMAKE_INSTALL_PREFIX}/lib")
    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    set                     (CMAKE_INSTALL_RPATH_USE_LINK_PATH true)
    # the RPATH to be used when installing, but only if it's not a system directory
    list                    (FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
                                                               "${CMAKE_INSTALL_PREFIX}/lib"
                                                               isSystemDir)
    if                      ("${isSystemDir}" STREQUAL         "-1")
      set                   (CMAKE_INSTALL_RPATH               "${CMAKE_INSTALL_PREFIX}/lib")
    endif                   ()
endif                       (${CMAKE_SYSTEM_NAME} MATCHES      "Darwin")

find_package                (MPI REQUIRED)
    set                     (libraries ${libraries} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})

# Include dirs
set                         (CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem")
include_directories         (${CMAKE_CURRENT_SOURCE_DIR}/include
                             SYSTEM ${MPI_INCLUDE_PATH}
	                     $ENV{SRT_PRIVATE_CONTEXT}/include
	                     $ENV{SRT_PUBLIC_CONTEXT}/include
	                     $ENV{ROOT_INC})

link_directories             (
  $ENV{SRT_PRIVATE_CONTEXT}/lib/Linux2.6-GCC-maxopt
  $ENV{SRT_PUBLIC_CONTEXT}/lib/Linux2.6-GCC-maxopt
  $ENV{ROOTSYS}/lib
  $ENV{}
)

# Libraries
set                         (libraries
                             ${libraries})

#                             uuid
#                             Gui Core Imt RIO Net Hist Graf Graf3d Gpad Tree TreePlayer Rint Postscript Matrix Physics MathCore Thread MultiProc
#	                     Minuit2 StandardRecord StandardRecordProxy StandardRecordNOvARwgt
#                             CAFAnaCore CAFAnaVars CAFAnaCuts)

#add_executable              (simpletest simple.cc ../disable_depman.cc)
#target_link_libraries       (simpletest ${libraries})
add_executable              (main main.cc)
target_link_libraries       (main ${libraries})
