link_directories(${GLEW_LIBRARY_DIRS})
add_definitions(${GLEW_DEFINITIONS})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/glew-cmake/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/cppformat)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/glm)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/rapidjson/include)
include_directories(${GLFW_INCLUDE_DIRS})

add_executable(rend main.cpp)

target_link_libraries(rend libglew_static)
target_link_libraries(rend ${GLFW_STATIC_LIBRARIES})
target_link_libraries(rend cppformat)

add_custom_target(copy_shaders)
set(SHADER_FILES particleshader_frag.glsl particleshader_vert.glsl particleshader_geom.glsl lineshader_vert.glsl lineshader_frag.glsl)
foreach(ShaderFile ${SHADER_FILES})
  add_custom_command(TARGET copy_shaders PRE_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${ShaderFile} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${ShaderFile})
endforeach()
add_dependencies(rend copy_shaders)
