cmake_minimum_required(VERSION 3.1)
project(MCIni)

find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

include_directories(include)

add_library(mcIniData SHARED include/Nucleon.h src/Nucleon.cpp include/BasicTypes.h include/EventInitialState.h src/EventInitialState.cpp)
target_link_libraries(mcIniData PRIVATE ${ROOT_LIBRARIES})
