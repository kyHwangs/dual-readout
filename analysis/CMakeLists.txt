project(analysis)

find_package(ROOT REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${FASTJET_DIR}/include
  ${HEPMC_DIR}/include
)
file(GLOB sources
  ${PROJECT_SOURCE_DIR}/src/*.cc
)
file(GLOB headers
  ${PROJECT_SOURCE_DIR}/include/*.h
)

include(${ROOT_USE_FILE})
add_executable(analysis analysis.cc ${sources} ${headers})
add_executable(full_ana full_ana.cc ${sources} ${headers})
add_executable(full_ana_LE full_ana_LE.cc ${sources} ${headers})
add_executable(JER JER.cc ${sources} ${headers})
add_executable(calib calib.cc ${sources} ${headers})
add_executable(make_script make_script.cc ${sources} ${headers})
add_executable(analysis_ele analysis_ele.cc ${sources} ${headers})

target_link_libraries(
  analysis
  ${HEPMC_DIR}/lib64/libHepMC3.so
  ${HEPMC_DIR}/lib64/libHepMC3rootIO.so
  rootIO
  ${ROOT_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

target_link_libraries(
  analysis_ele
  ${HEPMC_DIR}/lib64/libHepMC3.so
  ${HEPMC_DIR}/lib64/libHepMC3rootIO.so
  rootIO
  ${ROOT_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

target_link_libraries(
  full_ana
  ${HEPMC_DIR}/lib64/libHepMC3.so
  ${HEPMC_DIR}/lib64/libHepMC3rootIO.so
  rootIO
  ${ROOT_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

target_link_libraries(
  full_ana_LE
  ${HEPMC_DIR}/lib64/libHepMC3.so
  ${HEPMC_DIR}/lib64/libHepMC3rootIO.so
  rootIO
  ${ROOT_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

target_link_libraries(
  JER
  ${HEPMC_DIR}/lib64/libHepMC3.so
  ${HEPMC_DIR}/lib64/libHepMC3rootIO.so
  rootIO
  ${ROOT_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

target_link_libraries(
  calib
  ${HEPMC_DIR}/lib64/libHepMC3.so
  ${HEPMC_DIR}/lib64/libHepMC3rootIO.so
  rootIO
  ${ROOT_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

target_link_libraries(
  make_script
  ${HEPMC_DIR}/lib64/libHepMC3.so
  ${HEPMC_DIR}/lib64/libHepMC3rootIO.so
  rootIO
  ${ROOT_LIBRARIES}
  ${CMAKE_DL_LIBS}
  stdc++fs
)

set(
  analysis_SCRIPTS
)

foreach(_script ${analysis_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

install(TARGETS analysis_ele analysis full_ana_LE full_ana JER calib make_script DESTINATION bin)
