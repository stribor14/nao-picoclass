cmake_minimum_required(VERSION 2.8)
project(PicoModule)

find_package(qibuild)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR} )

option(MODULE_IS_REMOTE
  "ON - OFF"
  OFF)

aux_source_directory(./PICO SRC_PICO)
aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST "./pc_part.cpp")

if(MODULE_IS_REMOTE)
    add_definitions( " -DMODULE_IS_REMOTE")
    qi_create_bin(PicoModule ${SRC_PICO} ${SRC_LIST})
    qi_create_bin(pc_part pc_part.cpp APIwrappers.h)
    qi_use_lib(pc_part ALCOMMON ALPROXIES BOOST OPENCV2_CORE OPENCV2_IMGPROC)
else()
    qi_create_lib(PicoModule SHARED ${SRC_PICO} ${SRC_LIST} SUBFOLDER naoqi)
    qi_use_lib(PicoModule ALCOMMON ALPROXIES BOOST PTHREAD OPENCV2_CORE OPENCV2_IMGPROC)
endif()
