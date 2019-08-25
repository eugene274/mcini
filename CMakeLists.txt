cmake_minimum_required(VERSION 3.1)
project(McIniData)

find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

include_directories(include ${CMAKE_SOURCE_DIR})

ROOT_GENERATE_DICTIONARY(
        G__McIniData include/Nucleon.h include/EventInitialState.h include/URun.h include/UEvent.h include/UParticle.h 
        LINKDEF include/MCIni-LinkDef.h
)

add_library(McIniData SHARED G__McIniData.cxx src/URun.cxx src/UEvent.cxx src/UParticle.cxx src/Nucleon.cxx src/EventInitialState.cxx)
target_link_libraries(McIniData PRIVATE ${ROOT_LIBRARIES})

add_executable(JAMReader src/jam/JAMRUNReader.cxx src/jam/JAMReader.cxx src/jam/JAMPHReader.cxx)
target_link_libraries(JAMReader PUBLIC McIniData ${ROOT_LIBRARIES})
