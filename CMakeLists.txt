cmake_minimum_required(VERSION 3.1)
project(McIniData)

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED)

if (NOT ROOT_FOUND)
    message(WARNING "ROOT was not found with CMAKE_PREFIX_PATH...")
    list(APPEND CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake)
    find_package(ROOT REQUIRED)
endif ()

if (ROOT_USE_FILE)
    message("-- ROOT_USE_FILE ${ROOT_USE_FILE}")
    include(${ROOT_USE_FILE})
endif ()

include_directories(include ${CMAKE_SOURCE_DIR})

ROOT_GENERATE_DICTIONARY(
        G__McIniData include/Nucleon.h include/EventInitialState.h include/URun.h include/UEvent.h include/UParticle.h 
        LINKDEF include/MCIni-LinkDef.h
)

add_library(McIniData SHARED G__McIniData.cxx src/URun.cxx src/UEvent.cxx src/UParticle.cxx src/Nucleon.cxx src/EventInitialState.cxx)
target_link_libraries(McIniData PRIVATE ${ROOT_LIBRARIES})

add_executable(JAMReader src/jam/JAMRunInfo.cxx src/jam/JAMReader.cxx)
target_link_libraries(JAMReader PUBLIC McIniData ${ROOT_LIBRARIES})
