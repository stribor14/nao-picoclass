cmake_minimum_required(VERSION 2.8)
project(PicoModule)

find_package(qibuild)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR} )

set(CMAKE_CXX_FLAGS "-std=gnu++11")

option(MODULE_IS_REMOTE
  "ON - OFF"
  OFF)

aux_source_directory(. SRC_LIST)

if(MODULE_IS_REMOTE)
   add_definitions( " -DMODULE_IS_REMOTE ")
   qi_create_bin(PicoModule ${SRC_LIST})
else()
   qi_create_lib(PicoModule SHARED ${SRC_LIST} SUBFOLDER naoqi)
endif()

qi_use_lib(PicoModule ALCOMMON ALPROXIES BOOST)

