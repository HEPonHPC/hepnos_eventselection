cmake_minimum_required(VERSION 3.9)
project (eventselection)
enable_testing()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(MPI REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS C)
find_package(hepnos REQUIRED)

find_path(DIY_INCLUDE_DIRS diy/types.h)

# Include dirs
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include
                     ${DIY_INCLUDE_DIRS}
                     SYSTEM ${MPI_INCLUDE_PATH}
                     ${HDF5_INC}
                     ${HDF5_INCLUDE_DIRS}
                     $ENV{BOOST_INC}
                     $ENV{THALLIUM_INC}
                     $ENV{CEREAL_INC}
                     $ENV{DATAFORMAT_INC}
	                 $ENV{SRT_PRIVATE_CONTEXT}/include
	                 $ENV{SRT_PUBLIC_CONTEXT}/include
	                 $ENV{ROOT_INC})

link_directories ($ENV{SRT_PRIVATE_CONTEXT}/lib/Linux2.6-GCC-maxopt
                  $ENV{SRT_PUBLIC_CONTEXT}/lib/Linux2.6-GCC-maxopt
                  $ENV{ROOTSYS}/lib
                  $ENV{})

# Libraries
set (libraries ${libraries} 
               ${MPI_C_LIBRARIES}
               ${CMAKE_THREAD_LIBS_INIT}
               ${HDF5_C_LIBRARIES}
               uuid Gui Core Imt RIO Net Hist Graf Graf3d Gpad Tree TreePlayer Rint Postscript Matrix Physics MathCore Thread MultiProc
	           Minuit2 StandardRecord StandardRecordProxy StandardRecordNOvARwgt CAFAnaCore CAFAnaVars CAFAnaCuts
    )

add_executable(eventselection eventselection.cc disable_depman.cc)
target_link_libraries(eventselection ${libraries} hepnos ${Boost_SERIALIZATION_LIBRARY})

add_executable(new_eventselection new_eventselection.cc disable_depman.cc)
target_link_libraries(new_eventselection ${libraries} hepnos ${Boost_SERIALIZATION_LIBRARY})

add_library(testmain SHARED main.cc)
target_include_directories(testmain PRIVATE ${CMAKE_SOURCE_DIR})
add_executable(test_increment_at_most test_increment_at_most.cc)
target_link_libraries(test_increment_at_most PRIVATE testmain)
target_include_directories(test_increment_at_most PRIVATE ${CMAKE_SOURCE_DIR})

add_test(test_increment_at_most test_increment_at_most)
