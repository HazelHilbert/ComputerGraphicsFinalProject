cmake_minimum_required(VERSION 3.0)
project(FinalProject)

find_package(OpenGL REQUIRED)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_subdirectory(external)

include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glad-opengl-3.3/include/
	external/stb/
	external/tinygltf-2.9.3/
        FinalProject/
)

add_executable(scene
		FinalProject/scene.cpp
        FinalProject/render/shader.cpp
		FinalProject/animation.cpp
		FinalProject/animation.h
		FinalProject/terrain.cpp
		FinalProject/terrain.h
		FinalProject/utils.cpp
		FinalProject/utils.h
		FinalProject/sky.cpp
		FinalProject/sky.h
		FinalProject/box.cpp
		FinalProject/box.h
		external/stb/stb_perlin.h
		FinalProject/TerrainManager.cpp
		FinalProject/TerrainManager.h
		FinalProject/model.cpp
		FinalProject/model.h
		FinalProject/CityManager.cpp
		FinalProject/CityManager.h
		FinalProject/city.cpp
		FinalProject/city.h
		FinalProject/FoxManager.cpp
		FinalProject/FoxManager.h
)
target_link_libraries(scene
	${OPENGL_LIBRARY}
	glfw
	glad
)
