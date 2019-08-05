cmake_minimum_required(VERSION 3.1)
project(McIniData)

find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

include_directories(include)

ROOT_GENERATE_DICTIONARY(
        G__McIniData include/Nucleon.h include/EventInitialState.h include/URun.h include/UEvent.h include/UParticle.h 
        LINKDEF include/MCIni-LinkDef.h
)

add_library(McIniData SHARED G__McIniData src/URun.cxx src/UEvent.cxx src/UParticle.cxx src/Nucleon.cxx src/EventInitialState.cxx include/MCIni-LinkDef.h)
target_link_libraries(McIniData PRIVATE ${ROOT_LIBRARIES})

add_library(BaseConverter STATIC src/BaseConverter.cxx include/BaseConverter.h)
target_link_libraries(BaseConverter PRIVATE ${ROOT_LIBRARIES} McIniData)
