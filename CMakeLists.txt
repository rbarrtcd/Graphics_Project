cmake_minimum_required(VERSION 3.0)
project(lab4)

find_package(OpenGL REQUIRED)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_subdirectory(external)

include_directories(
		external/glfw-3.1.2/include/
		external/glm-0.9.7.1/
		external/tinygltf-2.9.3/
		external/glad-3.3/include/
		external/assimp/include
		external/
		render/
)


add_executable(test_scene
		test_scene.cpp
		render/shader.cpp
		entity.cpp
		light.cpp
		geometry.cpp
		utilities.cpp
		primitives.cpp
		skybox.cpp
		deferredShader.cpp
		)

target_link_libraries(test_scene
		glfw
		${OPENGL_LIBRARY}
		glad
		assimp

		)

add_executable(city
		city.cpp
		render/shader.cpp
		entity.cpp
		light.cpp
		geometry.cpp
		utilities.cpp
		primitives.cpp
		skybox.cpp
		deferredShader.cpp
		animationData.cpp
		)

target_link_libraries(city
		glfw
		${OPENGL_LIBRARY}
		glad
		assimp

		)
