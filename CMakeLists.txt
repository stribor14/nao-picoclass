cmake_minimum_required(VERSION 2.8)
set( CMAKE_C_COMPILER "/usr/bin/gcc-4.8" )
set( CMAKE_CXX_COMPILER "/usr/bin/g++-4.8" )
project(PicoModule)

find_package(qibuild)

set( CMAKE_CXX_FLAGS -DBOOST_SIGNALS_NO_DEPRECATION_WARNING )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR} )

option(MODULE_IS_REMOTE
  "ON - OFF"
  ON)

aux_source_directory(./PICO SRC_PICO)
aux_source_directory(. SRC_LIST)

if(MODULE_IS_REMOTE)
    add_definitions( " -DMODULE_IS_REMOTE")
    qi_create_bin(PicoModule ${SRC_PICO} ${SRC_LIST})
    qi_use_lib(PicoModule ALCOMMON ALPROXIES ALVISION ALEXTRACTOR BOOST PTHREAD OPENCV2_CORE OPENCV2_IMGPROC OPENCV2_HIGHGUI OPENCV2_CORE)
else()
    qi_create_lib(PicoModule SHARED ${SRC_PICO} ${SRC_LIST} SUBFOLDER naoqi)
    qi_use_lib(PicoModule ALCOMMON ALPROXIES ALVISION ALEXTRACTOR BOOST PTHREAD OPENCV2_CORE OPENCV2_IMGPROC)
endif()
