include_directories(${MPI_INCLUDE_PATH})
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/cppformat)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/rapidjson/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/glm)


add_executable(main main.cpp Controller.cpp Controller.h CellGroup.cpp CellGroup.h Particle.cpp Particle.h
  Cell.cpp Cell.h Client.cpp Client.h OutputDataPrinter.cpp OutputDataPrinter.h BasicCalculations.cpp BasicCalculations.h SDPDCalculations.h SDPDCalculations.cpp
  ParticleWienerRng.hpp ParticleWienerRng.cpp)
target_link_libraries(main ${MPI_LIBRARIES} cppformat)


add_custom_command(TARGET main POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ARGS
                       ${CMAKE_SOURCE_DIR}/config.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json)
