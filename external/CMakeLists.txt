if(MSVC AND NOT "${MSVC_VERSION}" LESS 1400)
	add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/MP>")
endif()

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Do not build Assimp tests")
set(ASSIMP_BUILD_SHARED_LIBS OFF CACHE BOOL "Build Assimp as static library")
add_subdirectory(assimp)
include_directories(assimp/include)


add_subdirectory (glad-3.3)

include_directories(
		glad-3.3/include/
)



### GLFW ###

add_subdirectory (glfw-3.1.2)

include_directories(
	glfw-3.1.2/include/GLFW/
)






if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	-lGL -lGLU -lXrandr -lXext -lX11 -lrt
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
