cmake_minimum_required(VERSION 3.16)
project(nai_detector)

# No ui_all or vis_all — pure batch mode
find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB sources ${CMAKE_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${CMAKE_SOURCE_DIR}/include/*.hh)

add_executable(nai_detector
    ${CMAKE_SOURCE_DIR}/main.cc
    ${sources}
    ${headers}
)
target_link_libraries(nai_detector ${Geant4_LIBRARIES})

set(NAI_SCRIPTS
  run1_co60.mac
  run2_co60.mac
)

foreach(_script ${NAI_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

add_custom_target(NaI_Detector DEPENDS nai_detector)
