cmake_minimum_required(VERSION 3.1)
project(MCIni)

find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

include_directories(include)


ROOT_GENERATE_DICTIONARY(
        G_MCIniData include/Nucleon.h include/EventInitialState.h
        LINKDEF include/MCIni-LinkDef.h
)

add_library(mcIniData SHARED G_MCIniData.cxx include/Nucleon.h src/Nucleon.cpp include/BasicTypes.h include/EventInitialState.h src/EventInitialState.cpp include/MCIni-LinkDef.h)
target_link_libraries(mcIniData PRIVATE ${ROOT_LIBRARIES})

add_library(BaseConverter STATIC src/BaseConverter.cpp include/BaseConverter.h)
target_link_libraries(BaseConverter PRIVATE ${ROOT_LIBRARIES} mcIniData)